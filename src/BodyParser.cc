/*
Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BodyParser.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/FileReader.h>
#include <tpau-cpp-kernal/LocationException.h>

#include "Body/AssignmentBody.h"
#include "Body/ChecksumBody.h"
#include "Body/DataBody.h"
#include "Body/EmptyBody.h"
#include "Body/LabelBody.h"
#include "Body/MacroBody.h"
#include "Body/MemoryBody.h"
#include "Expression/ConstantExpression.h"
#include "Expression/CurrentObjectExpression.h"
#include "Expression/ValueExpression.h"
#include "ExpressionNode.h"
#include "ExpressionParser.h"
#include "InstructionInvocation.h"
#include "TokenNode.h"

using namespace tpau::cpp_kernal;

const Symbol BodyParser::symbol_pc = Symbol(".pc");
const Token BodyParser::token_binary_file = Token(Token::DIRECTIVE, "binary_file");
const Token BodyParser::token_checksum = Token(Token::DIRECTIVE, "checksum");
const Token BodyParser::token_data = Token(Token::DIRECTIVE, "data");
const Token BodyParser::token_end = Token(Token::DIRECTIVE, "end");
const Token BodyParser::token_else = Token(Token::DIRECTIVE, "else");
const Token BodyParser::token_else_if = Token(Token::DIRECTIVE, "else_if");
const Token BodyParser::token_error = Token(Token::DIRECTIVE, "error");
const Token BodyParser::token_if = Token(Token::DIRECTIVE, "if");
const Token BodyParser::token_length = Token(Token::DIRECTIVE, "length");
const Token BodyParser::token_memory = Token(Token::DIRECTIVE, "memory");
const Token BodyParser::token_repeat = Token(Token::DIRECTIVE, "repeat");
const Token BodyParser::token_scope = Token(Token::DIRECTIVE, "scope");
const Token BodyParser::token_start = Token(Token::DIRECTIVE, "start");

// clang-format off
const std::unordered_map<Symbol, void (BodyParser::*)()> BodyParser::directive_parser_methods = {
    {token_binary_file.as_symbol(), &BodyParser::parse_binary_file},
    {token_checksum.as_symbol(), &BodyParser::parse_checksum},
    {token_data.as_symbol(), &BodyParser::parse_data},
    {token_else.as_symbol(), &BodyParser::parse_else},
    {token_else_if.as_symbol(), &BodyParser::parse_else_if},
    {token_error.as_symbol(), &BodyParser::parse_error},
    {token_if.as_symbol(), &BodyParser::parse_if},
    {token_memory.as_symbol(), &BodyParser::parse_memory},
    {token_repeat.as_symbol(), &BodyParser::parse_repeat},
    {token_scope.as_symbol(), &BodyParser::parse_scope},
};
// clang-format on

void BodyParser::setup(FileTokenizer& tokenizer) {
    tokenizer.add_literal(Token::colon_minus);
    tokenizer.add_literal(Token::colon_plus);
}

Body BodyParser::parse() {
    body = Body();
    current_body = &body;

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    throw LocationException(token.location, "unclosed symbol body");

                case Token::DIRECTIVE: {
                    if (auto visibility = VisibilityHelper::from_token(token)) {
                        handle_name(*visibility, tokenizer.expect(Token::NAME, TokenGroup::newline));
                        break;
                    }
                    parse_directive(token);
                    break;
                }

                case Token::NAME: {
                    handle_name(Visibility::ENTITY, token);
                    break;
                }

                case Token::INSTRUCTION:
                    if (allow_instructions()) {
                        parse_instruction(token);
                    }
                    else {
                        throw LocationException(token.location, "unexpected {}", token.type_name());
                    }
                    break;

                case Token::NEWLINE:
                    // ignore
                    break;

                case Token::PUNCTUATION:
                    if (token == Token::curly_close && !nesting.empty()) {
                        tokenizer.skip(Token::NEWLINE);
                        if (nesting.back()->is_if()) {
                            auto next_token = tokenizer.peek();
                            if (next_token == token_else || next_token == token_else_if) {
                                break;
                            }
                        }
                        pop_body();
                        current_body->append(nesting.back()->body());
                        nesting.pop_back();
                        break;
                    }
                    if (token == end_token) {
                        return body;
                    }
                    else if (token == Token::colon) {
                        parse_unnamed_label();
                    }
                    else {
                        if (allow_instructions() && cpu->uses_empty_mnemonic()) {
                            tokenizer.unget(token);
                            parse_instruction(Token(Token::NAME, token.location, Symbol()));
                        }
                        else {
                            throw LocationException(token.location, "unexpected {}", token.type_name());
                        }
                    }
                    break;

                default:
                    if (allow_instructions() && cpu->uses_empty_mnemonic()) {
                        tokenizer.unget(token);
                        parse_instruction(Token(Token::NAME, token.location, Symbol()));
                    }
                    else {
                        throw LocationException(token.location, "unexpected {}", token.type_name());
                    }
                    break;
            }
        } catch (LocationException& ex) {
            DiagnosticOutput::global.error(ex);
            tokenizer.skip_until(TokenGroup::newline);
        }
    }

    throw LocationException(Location(), "unclosed body"); // TODO: location
}

Expression BodyParser::get_label(const Location& location, Symbol name) const { return VariableExpression::create(location, name); }

void BodyParser::add_constant(Visibility visibility, const Token& name, const Expression& value) {
    if (visibility != Visibility::ENTITY) {
        if (!allow_assignment()) {
            throw LocationException(name.location, "unsupported visibility {}", visibility);
        }
        current_body->append(AssignmentBody::create(visibility, name.as_symbol(), value));
    }
}

Expression BodyParser::get_pc_label() {
    auto name = Symbol();

    if (current_body->is<EmptyBody>()) {
        return CurrentObjectExpression::create(tokenizer.current_location());
    }
    else {
        auto trailing_label = current_body->back();
        if (trailing_label && trailing_label->is<LabelBody>()) {
            name = trailing_label->as<LabelBody>()->name;
        }
        else {
            name = Symbol(std::format(".label_{}", next_label));
            next_label += 1;
            current_body->append(LabelBody::create(tokenizer.current_location(), name));
        }
    }

    auto trailing_label = current_body->back();
    if (trailing_label && trailing_label->is<LabelBody>()) {
        name = trailing_label->as<LabelBody>()->name;
    }
    else {
        name = Symbol(std::format(".label_{}", next_label));
        next_label += 1;
        current_body->append(LabelBody::create(tokenizer.current_location(), name));
    }
    return get_label(tokenizer.current_location(), name);
}

void BodyParser::parse_directive(const Token& directive) {
    auto it = directive_parser_methods.find(directive.as_symbol());
    if (it == directive_parser_methods.end() || ((directive == token_memory || directive == token_checksum) && !allow_memory())) {
        throw LocationException(directive.location, "unknown directive");
    }
    (this->*it->second)();
}

void BodyParser::parse_checksum() { current_body->append(ChecksumBody::parse(tokenizer)); }

void BodyParser::parse_data() {
    auto data = ExpressionParser(tokenizer).parse_list();
    current_body->append(data);
}

void BodyParser::parse_else() {
    if (nesting.empty()) {
        throw Exception(".else outside .if");
    }
    pop_body();
    push_clause(ValueExpression::create(tokenizer.current_location(), Value(true)));
    tokenizer.expect(Token::curly_open);
}

void BodyParser::parse_else_if() {
    if (nesting.empty()) {
        throw Exception(".else_if outside .if");
    }
    pop_body();
    push_clause(ExpressionParser(tokenizer).parse());
    tokenizer.expect(Token::curly_open);
}

void BodyParser::parse_if() {
    nesting.emplace_back(std::make_unique<IfNesting>());
    push_clause(ExpressionParser(tokenizer).parse());
    tokenizer.expect(Token::curly_open);
}

std::shared_ptr<Node> BodyParser::parse_instruction_argument(const Token& token) {
    switch (token.get_type()) {
        case Token::PUNCTUATION:
            if (cpu->uses_punctuation(token.as_symbol())) {
                return std::make_shared<TokenNode>(token);
            }
            break;

        case Token::KEYWORD:
            // TODO: check it's used by target.cpu, throw otherwise
            return std::make_shared<TokenNode>(token);

        case Token::DIRECTIVE:
        case Token::INSTRUCTION:
            throw LocationException(token.location, "unexpected {}", token.type_name());

        default:
            break;
    }

    tokenizer.unget(token);
    return std::make_shared<ExpressionNode>(ExpressionParser(tokenizer).parse());
}

void BodyParser::parse_label(Visibility visibility, const Token& name) { current_body->append(LabelBody::create(name.location, name.as_symbol(), visibility)); }

void BodyParser::parse_memory() {
    auto expression_parser = ExpressionParser(tokenizer);
    auto bank = ValueExpression::create({}, Value(uint64_t{0}));
    auto start_address = expression_parser.parse();
    tokenizer.expect(Token::comma);
    auto end_address = expression_parser.parse();
    auto token = tokenizer.next();
    if (token == Token::comma) {
        bank = start_address;
        start_address = end_address;
        end_address = expression_parser.parse();
    }
    else {
        tokenizer.unget(token);
    }
    // TODO: correct location
    current_body->append(MemoryBody::create({}, bank, start_address, end_address));
}

void BodyParser::parse_repeat() {
    auto token = tokenizer.next();

    auto variable = Symbol{};
    if (token.is_name()) {
        variable = token.as_symbol();
        tokenizer.expect(Token::comma);
    }
    else {
        tokenizer.unget(token);
    }

    auto start = std::optional<Expression>{};
    auto end = ExpressionParser(tokenizer).parse();
    token = tokenizer.next();
    if (token == Token::comma) {
        start = end;
        end = ExpressionParser(tokenizer).parse();
    }
    else {
        tokenizer.unget(token);
    }

    nesting.emplace_back(std::make_unique<RepeatNesting>(variable, start, end));
    push_body(NestingIndex(nesting.size() - 1, 0));
    tokenizer.expect(Token::curly_open);
}

void BodyParser::parse_assignment(Visibility visibility, const Token& name) {
    if (!allow_assignment()) {
        throw LocationException(name.location, "assignment only allowed in entities");
    }
    current_body->append(AssignmentBody::create(visibility, name.as_symbol(), ExpressionParser(tokenizer).parse()));
}

void BodyParser::parse_instruction(const Token& name) {
    std::vector<std::shared_ptr<Node>> arguments;

    Token token;
    while ((token = tokenizer.next()) && !token.is_newline()) {
        if (cpu->uses_braces() && token == Token::paren_open) {
            auto node = parse_instruction_argument(tokenizer.next());
            if (node->type() != Node::EXPRESSION) {
                // place '(' non-expression
                arguments.emplace_back(parse_instruction_argument(token));
                arguments.emplace_back(node);
            }
            else {
                auto token2 = tokenizer.next();
                if (token2.is_newline()) {
                    arguments.emplace_back(node);
                    // place trailing '(' expression
                    arguments.emplace_back(parse_instruction_argument(token));
                    arguments.emplace_back(node);
                    break;
                }
                else if (token2 == Token::paren_close) {
                    auto token3 = tokenizer.next();

                    if (token3.is_newline()) {
                        // place trailing '(' expression ')'
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));
                        break;
                    }

                    tokenizer.unget(token3);

                    if (!token3.is_punctuation() || cpu->uses_punctuation(token3.as_symbol())) {
                        // place '(' expression ')' that is not part of larger expression
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));
                    }
                    else {
                        // '(' expression ')' is part of larger expression
                        arguments.emplace_back(std::make_shared<ExpressionNode>(ExpressionParser(tokenizer).parse(std::dynamic_pointer_cast<ExpressionNode>(node)->expression)));
                    }
                }
                else {
                    tokenizer.unget(token2);
                    // place '(' expression that is not followed by ')'
                    arguments.emplace_back(parse_instruction_argument(Token::paren_open));
                    arguments.emplace_back(node);
                }
            }
        }
        else {
            arguments.emplace_back(parse_instruction_argument(token));
        }
    }

    auto invocation = InstructionInvocation(cpu, name, arguments, environment);

    if (invocation.uses_pc()) {
        invocation.add_pc(get_pc_label());
    }
    current_body->append(invocation.encode());
}

void BodyParser::parse_scope() {
    nesting.emplace_back(std::make_unique<ScopeNesting>());
    push_body(NestingIndex(nesting.size() - 1, 0));
    tokenizer.expect(Token::curly_open);
}

void BodyParser::push_body(const BodyParser::NestingIndex& body_index) {
    nesting_indices.emplace_back(body_index);
    current_body = get_body(body_index);
}

void BodyParser::pop_body() {
    if (nesting_indices.empty()) {
        throw Exception("internal error: closing outermost body");
    }
    nesting_indices.pop_back();
    if (nesting_indices.empty()) {
        current_body = &body;
    }
    else {
        current_body = get_body(nesting_indices.back());
    }
}

void BodyParser::push_clause(Expression condition) {
    auto if_nesting = nesting.back()->as_if();
    if (!if_nesting) {
        throw Exception("not inside .if");
    }
    if_nesting->add_clause(std::move(condition));
    push_body(NestingIndex(nesting.size() - 1, if_nesting->size() - 1));
}

SizeRange BodyParser::current_size() {
    auto size = body.size_range();

    for (auto& body_index : nesting_indices) {
        size += get_body(body_index)->size_range();
    }

    return size;
}

void BodyParser::parse_error() {
    auto message = tokenizer.expect(Token::STRING, TokenGroup::newline);
    auto token = tokenizer.next();
    Location location = message.location;
    if (token == Token::paren_open) {
        location = Location(tokenizer.expect(Token::STRING, TokenGroup::newline).as_string());
        token = tokenizer.next();
        if (token == Token::comma) {
            location.start.line_number = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
            token = tokenizer.next();
            if (token == Token::comma) {
                location.start.column = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
                token = tokenizer.next();
                if (token == Token::comma) {
                    location.end.line_number = location.start.line_number;
                    location.end.column = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
                    token = tokenizer.next();
                }
            }
        }
        if (token != Token::paren_close) {
            throw LocationException(token.location, "expected ')'");
        }
    }
    else {
        tokenizer.unget(token);
    }
    throw LocationException(location, message.as_string());
}

void BodyParser::parse_unnamed_label() { current_body->append(LabelBody::create(tokenizer.current_location())); }

void BodyParser::handle_name(Visibility visibility, const Token& name) {
    auto token = tokenizer.next();
    if (token == Token::colon) {
        parse_label(visibility, name);
    }
    else if (token == Token::equals) {
        parse_assignment(visibility, name);
    }
    else {
        tokenizer.unget(token);
        if (visibility != Visibility::ENTITY) {
            throw LocationException(name.location, "macro call can't have visibility");
        }

        std::vector<Expression> arguments;

        while (true) {
            token = tokenizer.next();
            if (!token || token.is_newline()) {
                break;
            }
            if (arguments.empty()) {
                tokenizer.unget(token);
            }
            else if (token != Token::comma) {
                throw LocationException(token.location, "expected ','");
            }
            arguments.emplace_back(tokenizer);
        }
        current_body->append(MacroBody::create(name.location, name.as_symbol(), arguments));
    }
}

void BodyParser::parse_binary_file() {
    auto filename = tokenizer.expect(Token::STRING, TokenGroup::newline);

    auto file_tokenizer = dynamic_cast<FileTokenizer*>(&tokenizer);
    if (!file_tokenizer) {
        throw LocationException(filename.location, "including from non-file source");
    }

    auto start = std::optional<size_t>{};
    auto length = std::optional<size_t>{};
    auto end = std::optional<size_t>{};

    Token directive;
    while (((directive = tokenizer.next())) && !directive.is_end_of_line()) {
        if (directive != token_start && directive != token_length && directive != token_end) {
            throw LocationException(directive.location, "expected .start, .length, or .end");
        }
        auto argument = tokenizer.expect(Token::Type::VALUE);
        if (!argument.is_unsigned()) {
            throw LocationException(directive.location, "expected unsigned");
        }
        if (directive == token_start) {
            if (start) {
                throw LocationException(directive.location, "duplicate .start");
            }
            start = argument.as_unsigned();
        }
        else if (directive == token_length) {
            if (length) {
                throw LocationException(directive.location, "duplicate .length");
            }
            length = argument.as_unsigned();
        }
        else if (directive == token_end) {
            if (end) {
                throw LocationException(directive.location, "duplicate .end");
            }
            length = argument.as_unsigned();
        }
    }

    if (length && end) {
        throw LocationException(filename.location, "specified both .end and .length");
    }

    if (!start) {
        start = 0;
    }
    if (end && *end < *start) {
        throw LocationException(filename.location, ".end less than .start");
    }
    if (length) {
        end = *start + *length - 1;
    }

    if (auto file_name = file_tokenizer->find_file(filename.as_symbol())) {
        auto data = FileReader::global.read_binary(file_name);
        if (start >= data.size() || end >= data.size()) {
            throw LocationException(filename.location, "specified range exceeds file data");
        }
        if (end) {
            data = data.substr(*start, *end - *start + 1);
        }
        else if (start > 0) {
            data = data.substr(*start);
        }
        auto elements = std::vector<DataBodyElement>{DataBodyElement{ValueExpression::create(filename.location, Value(data, true)), {}}};
        current_body->append(DataBody::create(elements));
    }
    else {
        throw LocationException(filename.location, "can't find file '{}'", filename);
    }
}
