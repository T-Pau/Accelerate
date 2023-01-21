/*
Assembler.cc --

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include <iomanip>
#include <memory>

#include "ParseException.h"
#include "FileReader.h"
#include "TokenNode.h"
#include "ExpressionParser.h"

bool Assembler::initialized = false;
symbol_t Assembler::symbol_opcode;
symbol_t Assembler::symbol_pc;
Token Assembler::token_brace_close;
Token Assembler::token_brace_open;
Token Assembler::token_colon;
Token Assembler::token_equals;


void Assembler::initialize() {
    if (!initialized) {
        symbol_opcode = SymbolTable::global.add(".opcode");
        symbol_pc = SymbolTable::global.add(".pc");
        token_brace_close = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(")"));
        token_brace_open = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("("));
        token_colon = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(":"));
        token_equals = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("="));

        initialized = true;
    }
}

AssemblerObject Assembler::parse(const std::string &file_name) {
    initialize();
    cpu.setup(tokenizer);
    ExpressionParser::setup(tokenizer);
    tokenizer.push(file_name);

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    break;

                case Token::DIRECTIVE:
                    parse_directive(token);
                    break;

                case Token::NAME: {
                    auto token2 = tokenizer.next();
                    if (token2 == token_colon) {
                        parse_label(token);
                    }
                    else if (token2 == token_equals) {
                        parse_assignment(token);
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

                default:
                    throw ParseException(token, "unexpected %s", token.type_name());
            }
        }
        catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
        }
    }

    return object;
}

void Assembler::parse_directive(const Token& directive) {
    // TODO: implement
    tokenizer.skip_until(TokenGroup::newline, true);
}

void Assembler::parse_instruction(const Token& name) {
    const auto& instruction = cpu.instruction(name.as_symbol());
    if (instruction == nullptr) {
        tokenizer.skip_until(TokenGroup::newline, true);
        throw ParseException(name, "unknown instruction '%s'", name.as_string().c_str());
    }

    std::vector<std::shared_ptr<Node>> arguments;

    Token token;
    while ((token = tokenizer.next()) && !token.is_newline()) {
        if (cpu.uses_braces() && token == token_brace_open) {
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

                    if (!token3.is_punctuation() || cpu.uses_punctuation(token3.as_symbol())) {
                        // place '(' expression ')' that is not part of larger expression
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));

                    }
                    else {
                        // '(' expression ')' is part of larger expression
                        arguments.emplace_back(ExpressionParser(tokenizer).parse(std::dynamic_pointer_cast<ExpressionNode>(node)));
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

    auto matches = cpu.match_addressing_modes(arguments);
    if (matches.empty()) {
        throw ParseException(name, "addressing mode not recognized");
    }

    // TODO: do in priority order (e. g. zero_page before absolute)
    bool found = false;
    for (const auto& match: matches ) {
        if (instruction->has_addressing_mode(match.addressing_mode)) {
            // TODO: check argument ranges
            found = true;

            auto environment = Environment(); // TODO: include outer environment

            const auto addressing_mode = cpu.addressing_mode(match.addressing_mode);
            const auto& notation = addressing_mode->notations[match.notation_index];
            auto it_notation = notation.elements.begin();
            auto it_arguments = arguments.begin();
            while (it_notation != notation.elements.end()) {
                if (it_notation->is_argument()) {
                    auto argument_type = addressing_mode->argument(it_notation->symbol);
                    switch (argument_type->type()) {
                        case ArgumentType::ENUM: {
                            if ((*it_arguments)->type() != Node::KEYWORD) {
                                throw ParseException((*it_arguments)->location, "enum argument is not an expression");
                            }
                            auto enum_type = dynamic_cast<const ArgumentTypeEnum *>(argument_type);
                            auto name = std::dynamic_pointer_cast<TokenNode>(*it_arguments)->as_symbol();
                            if (!enum_type->has_entry(name)) {
                                throw ParseException((*it_arguments)->location, "invalid enum argument");
                            }
                            environment.add(it_notation->symbol, std::make_shared<ExpressionNodeInteger>(enum_type->entry(name)));
                            break;
                        }

                        case ArgumentType::MAP: {
                            if ((*it_arguments)->type() != Node::EXPRESSION) {
                                throw ParseException((*it_arguments)->location, "map argument is not an expression");
                            }
                            if (std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->subtype() !=
                                ExpressionNode::INTEGER) {
                                throw ParseException((*it_arguments)->location, "map argument is not an integer");
                            }
                            auto map_type = dynamic_cast<const ArgumentTypeMap *>(argument_type);
                            auto value = std::dynamic_pointer_cast<ExpressionNodeInteger>(*it_arguments)->as_int();
                            if (!map_type->has_entry(value)) {
                                throw ParseException((*it_arguments)->location, "invalid map argument");
                            }
                            environment.add(it_notation->symbol, std::make_shared<ExpressionNodeInteger>(map_type->entry(value)));
                            break;
                        }

                        case ArgumentType::RANGE:
                            // TODO: check range, set size
                            if ((*it_arguments)->type() != Node::EXPRESSION) {
                                throw ParseException((*it_arguments)->location, "range argument is not an expression");
                            }
                            environment.add(it_notation->symbol, std::dynamic_pointer_cast<ExpressionNode>(*it_arguments));
                            break;
                    }
                }
                environment.add(symbol_opcode, std::make_shared<ExpressionNodeInteger>(instruction->opcode(match.addressing_mode)));

                it_notation++;
                it_arguments++;
            }

            auto first = true;
            std::vector<std::string> bytes;
            for (const auto& expression: addressing_mode->encoding) {
                auto value = ExpressionNode::evaluate(expression, environment);
                if (first) {
                    first = false;
                    std::cout << ".bytes ";
                }
                else {
                    std::cout << ", ";
                }
                if (value->subtype() == ExpressionNode::INTEGER) {
                    std::cout << "$" << std::setfill('0') << std::setw(2) << std::hex << std::dynamic_pointer_cast<ExpressionNodeInteger>(value)->as_int();
                }
                else {
                    std::cout << "...";
                }
            }
            std::cout << " ; " << name.as_string() << " " << SymbolTable::global[match.addressing_mode] << std::endl;

            // TODO: emit instruction
            break;
        }
    }
    if (!found) {
        throw ParseException(name, "invalid addressing mode for instruction");
    }
}

std::shared_ptr<Node> Assembler::parse_instruction_argument(const Token& token) {
    switch (token.get_type()) {
        case Token::PUNCTUATION:
        case Token::KEYWORD:
            // TODO: check it's used by CPU, throw otherwise
            return std::make_shared<TokenNode>(token);

        case Token::DIRECTIVE:
        case Token::INSTRUCTION:
            throw ParseException(token, "unexpected %s", token.type_name());

        default:
            tokenizer.unget(token);
            return ExpressionParser(tokenizer).parse();
    }
}


void Assembler::parse_label(const Token& name) {
    // TODO: implement

}

void Assembler::parse_assignment(const Token &name) {
    // TODO: implement
    tokenizer.skip_until(TokenGroup::newline, true);
}
