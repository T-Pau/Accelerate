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
#include "IntegerExpression.h"
#include "VariableExpression.h"

bool Assembler::initialized = false;
symbol_t Assembler::symbol_opcode;
symbol_t Assembler::symbol_pc;
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
        symbol_opcode = SymbolTable::global.add(".opcode");
        symbol_pc = SymbolTable::global.add(".pc");
        token_align = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".align"));
        token_brace_close = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(")"));
        token_brace_open = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("("));
        token_colon = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(":"));
        token_curly_brace_close = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("}"));
        token_curly_brace_open = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("{"));
        token_data = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".data"));
        token_equals = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("="));
        token_global = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".global"));
        token_local = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".local"));
        token_reserve = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".reserve"));
        token_section = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".section"));

        initialized = true;
    }
}

ObjectFile Assembler::parse(const std::string &file_name) {
    initialize();
    target.cpu.setup(tokenizer);
    ExpressionParser::setup(tokenizer);
    tokenizer.add_punctuations({"{", "}", "=", ":"});
    tokenizer.add_literal(token_align);
    tokenizer.add_literal(token_data);
    tokenizer.add_literal(token_global);
    tokenizer.add_literal(token_local);
    tokenizer.add_literal(token_reserve);
    tokenizer.add_literal(token_section);
    tokenizer.push(file_name);

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
                case Token::INTEGER:
                case Token::PREPROCESSOR:
                case Token::REAL:
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
                        if (target.cpu.uses_empty_mnemonic()) {
                            tokenizer.unget(token);
                            parse_instruction(Token(Token::NAME, token.location, static_cast<symbol_t >(0)));
                        }
                        else {
                            throw ParseException(token, "unexpected %s", token.type_name());
                        }
                    }
                    break;

                default:
                    if (target.cpu.uses_empty_mnemonic()) {
                        tokenizer.unget(token);
                        parse_instruction(Token(Token::NAME, token.location, static_cast<symbol_t >(0)));
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
    const auto& instruction = target.cpu.instruction(name.as_symbol());
    if (instruction == nullptr) {
        tokenizer.skip_until(TokenGroup::newline, true);
        throw ParseException(name, "unknown instruction '%s'", name.as_string().c_str());
    }

    std::vector<std::shared_ptr<Node>> arguments;

    Token token;
    while ((token = tokenizer.next()) && !token.is_newline()) {
        if (target.cpu.uses_braces() && token == token_brace_open) {
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

                    if (!token3.is_punctuation() || target.cpu.uses_punctuation(token3.as_symbol())) {
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

    auto matches = target.cpu.match_addressing_modes(arguments);
    if (matches.empty()) {
        throw ParseException(name, "addressing mode not recognized");
    }

    auto found = false;
    auto list = ExpressionList();
    size_t assumed_size = 0;

    for (const auto& match: matches) {
        if (!instruction->has_addressing_mode(match.addressing_mode)) {
            continue;
        }

        found = true;
        auto fits = true;
        auto current_assumed_size = std::numeric_limits<size_t>::max();

        auto environment = Environment(current_environment); // TODO: include outer environment

        const auto addressing_mode = target.cpu.addressing_mode(match.addressing_mode);
        const auto& notation = addressing_mode->notations[match.notation_index];
        auto it_notation = notation.elements.begin();
        auto it_arguments = arguments.begin();
        while (fits && it_notation != notation.elements.end()) {
            if (it_notation->is_argument()) {
                auto argument_type = addressing_mode->argument(it_notation->symbol);
                switch (argument_type->type()) {
                    case ArgumentType::ENUM: {
                        if ((*it_arguments)->type() != Node::KEYWORD) {
                            throw ParseException((*it_arguments)->get_location(), "enum argument is not a keyword");
                        }
                        auto enum_type = dynamic_cast<const ArgumentTypeEnum *>(argument_type);
                        auto value_name = std::dynamic_pointer_cast<TokenNode>(*it_arguments)->as_symbol();
                        if (!enum_type->has_entry(value_name)) {
                            throw ParseException((*it_arguments)->get_location(), "invalid enum argument");
                        }
                        environment.add(it_notation->symbol, std::make_shared<IntegerExpression>(enum_type->entry(value_name)));
                        break;
                    }

                    case ArgumentType::MAP: {
                        if ((*it_arguments)->type() != Node::EXPRESSION) {
                            throw ParseException((*it_arguments)->get_location(), "map argument is not an expression");
                        }
                        auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;
                        if (!expression->has_value()) {
                            throw ParseException((*it_arguments)->get_location(), "map argument is not an integer");
                        }
                        auto map_type = dynamic_cast<const ArgumentTypeMap *>(argument_type);
                        auto value = expression->value();
                        if (!map_type->has_entry(value)) {
                            throw ParseException((*it_arguments)->get_location(), "invalid map argument");
                        }
                        environment.add(it_notation->symbol, std::make_shared<IntegerExpression>(map_type->entry(value)));
                        break;
                    }

                    case ArgumentType::RANGE:
                        auto range_type = dynamic_cast<const ArgumentTypeRange *>(argument_type);

                        if ((*it_arguments)->type() != Node::EXPRESSION) {
                            throw ParseException((*it_arguments)->get_location(), "range argument is not an expression");
                        }
                        auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;
                        auto expression_size = expression->minimum_byte_size();
                        if (expression_size == 0) {
                            current_assumed_size = std::min(current_assumed_size, range_type->byte_size());
                        }
                        else if (expression->minimum_byte_size() > range_type->byte_size()) {
                            fits = false;
                            break;
                        }
                        expression->set_byte_size(range_type->byte_size());
                        environment.add(it_notation->symbol, expression);
                        break;
                }
            }

            it_notation++;
            it_arguments++;
        }

        if (!fits) {
            // TODO: store details
            continue;
        }

        environment.add(symbol_opcode, std::make_shared<IntegerExpression>(instruction->opcode(match.addressing_mode)));
        environment.add(symbol_pc, get_pc());

        auto current_list = ExpressionList();

        for (const auto& expression: addressing_mode->encoding) {
            try {
                auto value = Expression::evaluate(expression, environment);
                current_list.append(value);
            }
            catch (ParseException &ex) {
                fits = false;
                // TODO: store detail
            }
        }
        if (!fits) {
            continue;
        }

        if (current_assumed_size > assumed_size) {
            list = std::move(current_list);
            assumed_size = current_assumed_size;
            if (assumed_size == std::numeric_limits<size_t>::max()) {
                break;
            }
        }
    }

    if (!found) {
        throw ParseException(name, "invalid addressing mode for instruction");
    }

    if (assumed_size == 0) {
        throw ParseException(name, "arguments out of range"); // TODO: more details
    }

    current_object->append(list);
}

std::shared_ptr<Node> Assembler::parse_instruction_argument(const Token& token) {
    switch (token.get_type()) {
        case Token::PUNCTUATION:
            if (target.cpu.uses_punctuation(token.as_symbol())) {
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
            current_object->data.evaluate(*current_environment);
            current_environment = file_environment;
            break;
        }
        // TODO: parameters
        else if (token == token_align || token == token_reserve) {
            auto value = ExpressionParser(tokenizer).parse();

            value = Expression::evaluate(value, *file_environment);

            if (token == token_align) {
                if (!value->has_value()) {
                    throw ParseException(value->location, "alignment must be constant value");
                }
                current_object->alignment = value->value();
            }
            else {
                if (!value->has_value()) {
                    throw ParseException(value->location, "reservation must be constant value");
                }
                current_object->size = value->value();
            }
        }
        else {
            throw ParseException(token, "unexpected");
        }
    }

    if (current_section == 0) {
        throw ParseException(name, "symbol outside section");
    }
    if (current_object->empty()) {
        throw ParseException(name, "empty symbol");
    }

    // TODO: warn if reserved in saved section
    // TODO: error if data in unsaved section
}

std::shared_ptr<Expression> Assembler::get_pc() const {
    return BinaryExpression::create(std::make_shared<VariableExpression>(current_object->name), BinaryExpression::ADD, std::make_shared<IntegerExpression>(current_object->size));
}
