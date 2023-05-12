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

#include "BodyParser.h"
#include "ParseException.h"
#include "FileReader.h"
#include "TokenNode.h"
#include "ExpressionParser.h"
#include "VariableExpression.h"
#include "LabelBody.h"

bool Assembler::initialized = false;
Symbol Assembler::symbol_opcode;
Token Assembler::token_align;
Token Assembler::token_global;
Token Assembler::token_local;
Token Assembler::token_reserve;
Token Assembler::token_section;


void Assembler::initialize() {
    if (!initialized) {
        symbol_opcode = ".opcode";
        token_align = Token(Token::DIRECTIVE, ".align");
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
    BodyParser::setup(tokenizer);
    tokenizer.add_punctuations({"{", "}", "=", ":"});
    tokenizer.add_literal(token_align);
//    tokenizer.add_literal(token_data);
    tokenizer.add_literal(token_global);
    tokenizer.add_literal(token_local);
    tokenizer.add_literal(token_reserve);
    tokenizer.add_literal(token_section);
    tokenizer.push(file_name);

    object_file.target = &target;

    file_environment = std::make_shared<Environment>();

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    break;

                case Token::DIRECTIVE: {
                    auto visibility = visibility_value(token);
                    if (visibility != Object::OBJECT) {
                        auto name = tokenizer.next();
                        if (name.get_type() != Token::NAME) {
                            throw ParseException(name, "name expected");
                        }
                        auto token2 = tokenizer.next();
                        if (token2 == Token::equals) {
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
                    if (token2 == Token::equals) {
                        parse_assignment(Object::OBJECT, token);
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

void Assembler::add_constant(Object::Visibility visibility, const Token& name, Expression value) {
    value.evaluate(*file_environment);
    switch (visibility) {
        case Object::OBJECT:
            break;
        case Object::LOCAL:
        case Object::GLOBAL:
            object_file.add_constant(name.as_symbol(), visibility, value);
            break;
    }
    file_environment->add(name.as_symbol(), value);
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
        return Object::OBJECT;
    }
}

void Assembler::parse_symbol(Object::Visibility visibility, const Token &name) {
    auto object = object_file.create_object(current_section, visibility, name);

    while (true) {
        auto token = tokenizer.next();
        if (token.is_newline()) {
            tokenizer.unget(token);
            break;
        }
        else if (token == Token::curly_open) {
            // TODO: error if .reserved
            object->body = BodyParser(tokenizer, object->name.as_symbol(), target.cpu, &object_file, file_environment).parse();
            break;
        }
        // TODO: parameters
        else if (token == token_align || token == token_reserve) {
            auto expression = ExpressionParser(tokenizer).parse();

            expression.evaluate(*file_environment);
            auto value = expression.value();

            if (token == token_align) {
                if (!value.has_value() || !value->is_unsigned()) {
                    throw ParseException(expression.location(), "alignment must be constant unsigned integer");
                }
                object->alignment = value->unsigned_value();
            }
            else {
                if (!value.has_value() || !value->is_unsigned()) {
                    throw ParseException(expression.location(), "reservation must be constant expression");
                }
                object->size = value->unsigned_value();
            }
        }
        else {
            throw ParseException(token, "unexpected");
        }
    }

    if (current_section.empty()) {
        throw ParseException(name, "symbol outside section");
    }
    if (object->empty()) {
        throw ParseException(name, "empty symbol");
    }

    // TODO: warn if reserved in saved section
    // TODO: error if data in unsaved section
}
