/*
BodyParser.cc --

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

#include <utility>

#include "ParseException.h"
#include "FileReader.h"
#include "ExpressionParser.h"
#include "ValueExpression.h"
#include "ExpressionNode.h"
#include "InstructionEncoder.h"
#include "LabelExpression.h"
#include "TokenNode.h"
#include "MemoryBody.h"
#include "ObjectNameExpression.h"

const Symbol BodyParser::symbol_pc = Symbol(".pc");
const Token BodyParser::token_data = Token(Token::DIRECTIVE, "data");
const Token BodyParser::token_else = Token(Token::DIRECTIVE, "else");
const Token BodyParser::token_else_if = Token(Token::DIRECTIVE, "else_if");
const Token BodyParser::token_end = Token(Token::DIRECTIVE, "end");
const Token BodyParser::token_error = Token(Token::DIRECTIVE, "error");
const Token BodyParser::token_if = Token(Token::DIRECTIVE, "if");
const Token BodyParser::token_memory = Token(Token::DIRECTIVE, "memory");
const Token BodyParser::token_scope = Token(Token::DIRECTIVE, "scope");

const std::unordered_map<Symbol, void (BodyParser::*)()> BodyParser::directive_parser_methods = {
        {token_data.as_symbol(), &BodyParser::parse_data},
        {token_else.as_symbol(), &BodyParser::parse_else},
        {token_else_if.as_symbol(), &BodyParser::parse_else_if},
        {token_end.as_symbol(), &BodyParser::parse_end},
        {token_error.as_symbol(), &BodyParser::parse_error},
        {token_if.as_symbol(), &BodyParser::parse_if},
        {token_memory.as_symbol(), &BodyParser::parse_memory},
        {token_scope.as_symbol(), &BodyParser::parse_scope}
};

void BodyParser::setup(FileTokenizer &tokenizer) {
    VisibilityHelper::setup(tokenizer, true);
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
                    throw ParseException(token, "unclosed symbol body");

                case Token::DIRECTIVE: {
                    auto visibility = VisibilityHelper::from_token(token);
                    if (visibility) {
                        handle_name(*visibility, tokenizer.expect(Token::NAME, TokenGroup::newline));
                        break;
                    }
                    parse_directive(token);
                    break;
                }

                case Token::NAME: {
                    handle_name(Visibility::SCOPE, token);
                    break;
                }

                case Token::INSTRUCTION:
                    if (allow_instructions()) {
                        parse_instruction(token);
                    }
                    else {
                        throw ParseException(token, "unexpected %s", token.type_name());
                    }
                    break;

                case Token::NEWLINE:
                    // ignore
                    break;

                case Token::PUNCTUATION:
                    if (token == end_token) {
                        // TODO: move this to Object or ObjectFile.
                        if (entity) {
                            // TODO: check that ifs is empty
                            EvaluationResult result;
                            // Evaluate twice to resolve forward label references.
                            body.evaluate(EvaluationContext(result, entity));
                            body.evaluate(EvaluationContext(result, entity));
                            // TODO: process result
                        }
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
                            throw ParseException(token, "unexpected %s", token.type_name());
                        }
                    }
                    break;

                default:
                    if (allow_instructions() && cpu->uses_empty_mnemonic()) {
                        tokenizer.unget(token);
                        parse_instruction(Token(Token::NAME, token.location, Symbol()));
                    }
                    else {
                        throw ParseException(token, "unexpected %s", token.type_name());
                    }
                    break;
            }
        }
        catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
            tokenizer.skip_until(TokenGroup::newline);
        }
    }

    throw ParseException(Location(), "unclosed body"); // TODO: location
}

void BodyParser::add_constant(Visibility visibility, Token name, const Expression& value) {
    if (visibility != Visibility::SCOPE) {
        if (!entity) {
            throw ParseException(name, "unsupported visibility");
        }
        entity->owner->add_constant(std::make_unique<ObjectFile::Constant>(entity->owner, name, visibility, value));
    }
    environment->add(name.as_symbol(), value);
}


std::shared_ptr<Label> BodyParser::get_label(bool& is_anonymous) {
    auto trailing_label = current_body->back();
    if (trailing_label && trailing_label->is_label()) {
        is_anonymous = false;
        return trailing_label->as_label()->label;
    }
    next_label += 1;
    auto name = Symbol(".label_" + std::to_string(next_label));
    is_anonymous = true;
    return std::make_shared<Label>(name, current_size());
}


Expression BodyParser::get_pc(std::shared_ptr<Label> label) const {
    return {ObjectNameExpression::create(entity->as_object()), Expression::BinaryOperation::ADD, Expression(Location(), entity, std::move(label))};
}


void BodyParser::parse_directive(const Token& directive) {
    auto it = directive_parser_methods.find(directive.as_symbol());
    if (it == directive_parser_methods.end() || (directive == token_memory && !allow_memory())) {
        throw ParseException(directive, "unknown directive");
    }
    (this->*it->second)();
}


void BodyParser::parse_data() {
    EvaluationResult result;
    auto data = ExpressionParser(tokenizer).parse_list();
    current_body->append(data);
}

void BodyParser::parse_else() {
    if (nesting.empty()) {
        throw Exception(".else outside .if");
    }
    pop_body();
    push_clause(Expression(Value(true)));
}

void BodyParser::parse_else_if() {
    if (nesting.empty()) {
        throw Exception(".else_if outside .if");
    }
    pop_body();
    push_clause(ExpressionParser(tokenizer).parse());
}

void BodyParser::parse_end() {
    if (nesting.empty()) {
        throw Exception(".end outside .if or .scope");
    }
    pop_body();
    current_body->append(nesting.back()->body());
    nesting.pop_back();
}

void BodyParser::parse_if() {
    nesting.emplace_back(std::make_unique<IfNesting>());
    push_clause(ExpressionParser(tokenizer).parse());
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
            throw ParseException(token, "unexpected %s", token.type_name());

        default:
            break;
    }

    tokenizer.unget(token);
    return std::make_shared<ExpressionNode>(ExpressionParser(tokenizer).parse());
}


void BodyParser::parse_label(Visibility visibility, const Token& name) {
    auto label = std::make_shared<Label>(name.as_symbol(), SizeRange(current_body->size_range()));
    current_body->append(Body(label));
    add_constant(visibility, name, get_pc(label));
}


void BodyParser::parse_memory() {
    auto expression_parser = ExpressionParser(tokenizer);
    auto bank = Expression(uint64_t(0));
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
    current_body->append(Body(bank, start_address, end_address));
}

void BodyParser::parse_assignment(Visibility visibility, const Token &name) {
    if (!entity) {
        throw ParseException(name, "assignment only allowed in entities");
    }
    current_body->append(Body(visibility, name.as_symbol(), ExpressionParser(tokenizer).parse()));
}

void BodyParser::parse_instruction(const Token &name) {
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

    auto encoder = InstructionEncoder(cpu);

    auto is_anonymous_label = false;
    auto label = get_label(is_anonymous_label);
    auto instruction = Body();
    {
        auto instruction_environment = std::make_shared<Environment>();
        instruction_environment->add(symbol_pc, get_pc(label));
        instruction_environment->add(Symbol(".label_offset(" + label->name.str() + ")"), Expression(Location(), entity_name(), label));
        instruction = encoder.encode(name, arguments, instruction_environment, current_size());
    }
    if (is_anonymous_label) {
        if (label.use_count() > 1) {
            auto combined_body = Body(label);
            combined_body.append(instruction);
            instruction = combined_body;
        }
        else {
            next_label -= 1;
        }
    }

    //EvaluationResult result;
    //instruction.evaluate(result, entity, environment, current_size(), nesting.empty());
    // TODO: process result

    current_body->append(instruction);
}


void BodyParser::parse_scope() {
    nesting.emplace_back(std::make_unique<ScopeNesting>());
    push_body(NestingIndex(nesting.size() - 1, 0));
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

    for (auto& body_index: nesting_indices) {
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
            location.start_line_number = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
            token = tokenizer.next();
            if (token == Token::comma) {
                location.start_column = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
                token = tokenizer.next();
                if (token == Token::comma) {
                    location.end_column = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
                    token = tokenizer.next();
                }
            }
        }
        if (token != Token::paren_close) {
            throw ParseException(token, "expected ')'");
        }
    }
    else {
        tokenizer.unget(token);
    }
}

void BodyParser::parse_unnamed_label() {
    current_body->append(Body(std::make_shared<Label>(current_size())));
}

void BodyParser::handle_name(Visibility visibility, Token name) {
    auto token = tokenizer.next();
    if (token == Token::colon) {
        parse_label(visibility, name);
    }
    else if (token == Token::equals) {
        parse_assignment(visibility, name);
    }
    else {
        tokenizer.unget(token);
        if (visibility != Visibility::SCOPE) {
            throw ParseException(name, "macro call can't have visibility");
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
                throw ParseException(token, "expected ','");
            }
            arguments.emplace_back(tokenizer);
        }
        current_body->append(Body(name, arguments));
    }
}
