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
Token Assembler::token_address;
Token Assembler::token_align;
Token Assembler::token_macro;
Token Assembler::token_reserve;
Token Assembler::token_section;
Token Assembler::token_target;
Token Assembler::token_visibility;


void Assembler::initialize() {
    if (!initialized) {
        symbol_opcode = ".opcode";
        token_address = Token(Token::DIRECTIVE, "address");
        token_align = Token(Token::DIRECTIVE, "align");
        token_macro = Token(Token::DIRECTIVE, "macro");
        token_reserve = Token(Token::DIRECTIVE, "reserve");
        token_section = Token(Token::DIRECTIVE, "section");
        token_target = Token(Token::DIRECTIVE, "target");
        token_visibility = Token(Token::DIRECTIVE, "visibility");

        initialized = true;
    }
}

std::shared_ptr<ObjectFile> Assembler::parse(Symbol file_name) {
    initialize();
    if (target) {
        target->cpu->setup(tokenizer);
    }
    ExpressionParser::setup(tokenizer);
    BodyParser::setup(tokenizer);
    tokenizer.add_punctuations({"{", "}", "=", ":"});
    tokenizer.push(file_name);

    object_file = std::make_shared<ObjectFile>();
    object_file->target = target;

    file_environment = std::make_shared<Environment>(object_file->private_environment);
    
    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    break;

                case Token::DIRECTIVE:
                    parse_directive(token);
                    break;

                case Token::NAME:
                    parse_name(current_visibility, token);
                    break;

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

    object_file->evaluate(file_environment);

    return object_file;
}


void Assembler::parse_assignment(Visibility visibility, const Token &name) {
    auto value = ExpressionParser(tokenizer).parse();
    object_file->add_constant(std::make_unique<ObjectFile::Constant>(name, visibility, value));
}

void Assembler::parse_section() {
    auto token = tokenizer.expect(Token::NAME, TokenGroup::newline);

    if (!target) {
        throw ParseException(token, "no target specified");
    }
    if (!target->map.has_section(token.as_symbol())) {
        throw ParseException(token, "unknown section");
    }
    current_section = token.as_symbol();
}

void Assembler::parse_symbol(Visibility visibility, const Token &name) {
    if (!target) {
        tokenizer.skip_until(Token::curly_close, true);
        throw ParseException(name, "no target specified");
    }
    auto object = object_file->create_object(current_section, visibility, name);

    while (true) {
        auto token = tokenizer.next();
        if (token.is_newline()) {
            tokenizer.unget(token);
            break;
        }
        else if (token == Token::curly_open) {
            // TODO: error if .reserved
            object->body = BodyParser(tokenizer, object, target->cpu, file_environment).parse();
            break;
        }
        // TODO: parameters
        else if (token == token_address) {
            object->address = Address(tokenizer, file_environment);
        }
        else if (token == token_align || token == token_reserve) {
            auto expression = ExpressionParser(tokenizer).parse();

            expression.evaluate(file_environment);
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
                object->reservation = value->unsigned_value();
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

void Assembler::parse_directive(const Token& directive) {
    auto visibility = VisibilityHelper::from_token(directive);
    if (visibility) {
        auto name = tokenizer.next();
        if (name == token_macro) {
            parse_macro(*visibility);
        }
        else if (name.get_type() != Token::NAME) {
            throw ParseException(name, "name expected");
        }
        parse_name(*visibility, name);
    }
    else if (directive == token_macro) {
        parse_macro(current_visibility);
    }
    else if (directive == token_section) {
        parse_section();
    }
    else if (directive == token_target) {
        parse_target();
    }
    else if (directive == token_visibility) {
        auto name = tokenizer.expect(Token::NAME);
        visibility = VisibilityHelper::from_token(name);
        if (!visibility) {
            throw ParseException(name, "unknown visibility");
        }
        current_visibility = *visibility;
    }
    else {
        throw ParseException(directive, "unknown directive");
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);

}

void Assembler::parse_target() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);
    // TODO: check that target comes first
    auto& new_target = Target::get(name.as_symbol());
    if (target) {
        // TODO: check that targets are compatible
    }
    else {
        target = &new_target;
        object_file->target = target;
    }
}


void Assembler::parse_name(Visibility visibility, const Token& name) {
    auto token = tokenizer.next();
    if (token == Token::equals) {
        parse_assignment(visibility, name);
        return;
    }
    else if (token == Token::paren_open) {
        auto arguments = Callable::Arguments(tokenizer);
        tokenizer.expect(Token::paren_close);
        tokenizer.expect(Token::equals);

        auto definition = ExpressionParser(tokenizer).parse();
        object_file->add_function(std::make_unique<Function>(name, visibility, arguments, definition));
    }
    else {
        tokenizer.unget(token);
        parse_symbol(visibility, name);
    }
}

void Assembler::parse_macro(Visibility visibility) {
    auto name = tokenizer.expect(Token::NAME);
    auto arguments = Callable::Arguments(tokenizer);
    tokenizer.expect(Token::curly_open);
    auto body = BodyParser(tokenizer, object_file->target->cpu).parse(); // TODO: proper mode for macros
    object_file->add_macro(std::make_unique<Macro>(name, visibility, arguments, body));
}
