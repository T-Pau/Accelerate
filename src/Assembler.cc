/*
Assembler.cc --

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

#include "Assembler.h"

#include <memory>
#include <utility>

#include "ParseException.h"
#include "FileReader.h"
#include "TokenNode.h"
#include "ExpressionParser.h"
#include "ExpressionNode.h"
#include "ValueExpression.h"
#include "VariableExpression.h"
#include "InstructionEncoder.h"

bool Assembler::initialized = false;
Symbol Assembler::symbol_opcode;
Symbol Assembler::symbol_pc;
Token Assembler::token_align;
Token Assembler::token_brace_close;
Token Assembler::token_brace_open;
Token Assembler::token_colon;
Token Assembler::token_curly_brace_close;
Token Assembler::token_curly_brace_open;
Token Assembler::token_data;
Token Assembler::token_equals;
Token Assembler::token_global;
Token Assembler::token_local;
Token Assembler::token_reserve;
Token Assembler::token_section;


void Assembler::initialize() {
    if (!initialized) {
        symbol_opcode = ".opcode";
        symbol_pc = ".pc";
        token_align = Token(Token::DIRECTIVE, ".align");
        token_brace_close = Token(Token::PUNCTUATION, ")");
        token_brace_open = Token(Token::PUNCTUATION, "(");
        token_colon = Token(Token::PUNCTUATION, ":");
        token_curly_brace_close = Token(Token::PUNCTUATION, "}");
        token_curly_brace_open = Token(Token::PUNCTUATION, "{");
        token_data = Token(Token::DIRECTIVE, ".data");
        token_equals = Token(Token::PUNCTUATION, "=");
        token_global = Token(Token::DIRECTIVE, ".global");
        token_local = Token(Token::DIRECTIVE, ".local");
        token_reserve = Token(Token::DIRECTIVE, ".reserve");
        token_section = Token(Token::DIRECTIVE, ".section");

        initialized = true;
    }
}

ObjectFile Assembler::parse(Symbol file_name) {
    initialize();
    target.cpu->setup(tokenizer);
    ExpressionParser::setup(tokenizer);
    tokenizer.add_punctuations({"{", "}", "=", ":"});
    tokenizer.add_literal(token_align);
    tokenizer.add_literal(token_data);
    tokenizer.add_literal(token_global);
    tokenizer.add_literal(token_local);
    tokenizer.add_literal(token_reserve);
    tokenizer.add_literal(token_section);
    tokenizer.push(file_name);

    object_file.target = &target;

    file_environment = std::make_shared<Environment>();
    current_environment = file_environment;

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    break;

                case Token::DIRECTIVE: {
                    auto visibility = visibility_value(token);
                    if (visibility != Object::NONE) {
                        auto name = tokenizer.next();
                        if (name.get_type() != Token::NAME) {
                            throw ParseException(name, "name expected");
                        }
                        auto token2 = tokenizer.next();
                        if (token2 == token_equals) {
                            parse_assignment(visibility, name);
                        }
                        else {
                            tokenizer.unget(token2);
                            parse_symbol(visibility, name);
                        }
                    }
                    else if (token == token_section) {
                        parse_section();
                    }
                    else {
                        throw ParseException(token, "unknown directive");
                    }
                    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
                    break;
                }

                case Token::NAME: {
                    auto token2 = tokenizer.next();
                    if (token2 == token_equals) {
                        parse_assignment(Object::NONE, token);
                        break;
                    }
                    else {
                        throw ParseException(token2, "unexpected");
                    }
                }

                case Token::NEWLINE:
                    // ignore
                    break;

                case Token::INSTRUCTION:
                    throw ParseException(token, "instruction not allowed outside symbol");

                case Token::PUNCTUATION:
                case Token::VALUE:
                case Token::PREPROCESSOR:
                case Token::STRING:
                case Token::KEYWORD:
                    throw ParseException(token, "unexpected %s", token.type_name());
            }
        }
        catch (ParseException &ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
            tokenizer.skip_until(TokenGroup::newline, true);
        }
    }

    object_file.evaluate(*file_environment);

    return object_file;
}

void Assembler::parse_symbol_body() {
    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    throw ParseException(token, "unclosed symbol body");

                case Token::DIRECTIVE:
                    parse_directive(token);
                    break;

                case Token::NAME: {
                    auto token2 = tokenizer.next();
                    if (token2 == token_colon) {
                        parse_label(Object::NONE, token);
                    }
                    else if (token2 == token_equals) {
                        parse_assignment(Object::NONE, token);
                    }
                    else {
                        tokenizer.unget(token2);
                        throw ParseException(token, "unexpected name");
                    }
                    break;
                }

                case Token::INSTRUCTION:
                    parse_instruction(token);
                    break;

                case Token::NEWLINE:
                    // ignore
                    break;

                case Token::PUNCTUATION:
                    if (token == token_curly_brace_close) {
                        return;
                    }
                    else {
                        if (target.cpu->uses_empty_mnemonic()) {
                            tokenizer.unget(token);
                            parse_instruction(Token(Token::NAME, token.location, Symbol()));
                        }
                        else {
                            throw ParseException(token, "unexpected %s", token.type_name());
                        }
                    }
                    break;

                default:
                    if (target.cpu->uses_empty_mnemonic()) {
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
        }
    }
}

void Assembler::parse_directive(const Token& directive) {
    // TODO: use table of directive parsing methods
    if (directive == token_data) {
        current_object->append(ExpressionParser(tokenizer).parse_list());
    }
    else {
        // TODO: implement
        tokenizer.skip_until(TokenGroup::newline, true);
    }
}

void Assembler::parse_instruction(const Token& name) {
    std::vector<std::shared_ptr<Node>> arguments;

    Token token;
    while ((token = tokenizer.next()) && !token.is_newline()) {
        if (target.cpu->uses_braces() && token == token_brace_open) {
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
                else if (token2 == token_brace_close) {
                    auto token3 = tokenizer.next();

                    if (token3.is_newline()) {
                        // place trailing '(' expression ')'
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));
                        break;
                    }

                    tokenizer.unget(token3);

                    if (!token3.is_punctuation() || target.cpu->uses_punctuation(token3.as_symbol())) {
                        // place '(' expression ')' that is not part of larger expression
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));

                    }
                    else {
                        // '(' expression ')' is part of larger expression
                        arguments.emplace_back(std::make_shared<ExpressionNode>(ExpressionParser(tokenizer).parse(std::dynamic_pointer_cast<Expression>(node))));
                    }
                }
                else {
                    tokenizer.unget(token2);
                    // place '(' expression that is not followed by ')'
                    arguments.emplace_back(parse_instruction_argument(token_brace_open));
                    arguments.emplace_back(node);
                }
            }
        }
        else {
            arguments.emplace_back(parse_instruction_argument(token));
        }
    }

    auto encoder = InstructionEncoder(target);
    // TODO: hack, needs proper solution that works if size is unknown
    auto instruction_environment = std::make_shared<Environment>(current_environment);
    auto offset = current_object->data ? current_object->data->size() : 0;
    if (offset.has_value()) {
        instruction_environment->add(symbol_pc, std::make_shared<BinaryExpression>(std::make_shared<VariableExpression>(current_object->name.as_symbol()), BinaryExpression::ADD, std::make_shared<ValueExpression>(Value(*offset))));
    }
    auto instruction = encoder.encode(name, arguments, instruction_environment);

    current_object->append(instruction);
}

std::shared_ptr<Node> Assembler::parse_instruction_argument(const Token& token) {
    switch (token.get_type()) {
        case Token::PUNCTUATION:
            if (target.cpu->uses_punctuation(token.as_symbol())) {
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


void Assembler::parse_label(Object::Visibility visibility, const Token& name) {
    add_constant(visibility, name, get_pc());
}

void Assembler::add_constant(Object::Visibility visibility, const Token& name, std::shared_ptr<Expression> value) {
    auto evaluated_value = Expression::evaluate(std::move(value), *current_environment);
    switch (visibility) {
        case Object::NONE:
            break;
        case Object::LOCAL:
        case Object::GLOBAL:
            object_file.add_constant(name.as_symbol(), visibility, evaluated_value);
            break;
    }
    current_environment->add(name.as_symbol(), evaluated_value);
}

void Assembler::parse_assignment(Object::Visibility visibility, const Token &name) {
    add_constant(visibility, name, ExpressionParser(tokenizer).parse());
}

void Assembler::parse_section() {
    auto token = tokenizer.expect(Token::NAME, TokenGroup::newline);

    if (!target.map.has_section(token.as_symbol())) {
        throw ParseException(token, "unknown section");
    }
    current_section = token.as_symbol();
}

Object::Visibility Assembler::visibility_value(const Token& token) {
    if (token == token_local) {
        return Object::LOCAL;
    }
    else if (token == token_global) {
        return Object::GLOBAL;
    }
    else {
        return Object::NONE;
    }
}

void Assembler::parse_symbol(Object::Visibility visibility, const Token &name) {
    current_object = object_file.create_object(current_section, visibility, name);

    while (true) {
        auto token = tokenizer.next();
        if (token.is_newline()) {
            tokenizer.unget(token);
            break;
        }
        else if (token == token_curly_brace_open) {
            // TODO: error if .reserved
            current_environment = std::make_shared<Environment>(file_environment);
            parse_symbol_body();
            current_object->evaluate(*current_environment);
            current_environment = file_environment;
            break;
        }
        // TODO: parameters
        else if (token == token_align || token == token_reserve) {
            auto expression = ExpressionParser(tokenizer).parse();

            expression = Expression::evaluate(expression, *file_environment);
            auto value = expression->value();

            if (token == token_align) {
                if (!value.has_value() || !value->is_unsigned()) {
                    throw ParseException(expression->location, "alignment must be constant unsigned integer");
                }
                current_object->alignment = value->unsigned_value();
            }
            else {
                if (!value.has_value() || !value->is_unsigned()) {
                    throw ParseException(expression->location, "reservation must be constant expression");
                }
                current_object->size = value->unsigned_value();
            }
        }
        else {
            throw ParseException(token, "unexpected");
        }
    }

    if (current_section.empty()) {
        throw ParseException(name, "symbol outside section");
    }
    if (current_object->empty()) {
        throw ParseException(name, "empty symbol");
    }

    // TODO: warn if reserved in saved section
    // TODO: error if data in unsaved section
}

std::shared_ptr<Expression> Assembler::get_pc() const {
    return BinaryExpression::create(std::make_shared<VariableExpression>(current_object->name), BinaryExpression::ADD, std::make_shared<ValueExpression>(current_object->size));
}
