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
#include "ParseException.h"
#include "FileReader.h"

bool Assembler::initialized = false;
Token Assembler::token_colon;
Token Assembler::token_equals;


void Assembler::initialize() {
    if (!initialized) {
        token_colon = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(":"));
        token_equals = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("="));

        initialized = true;
    }
}

AssemblerObject Assembler::parse(const std::string &file_name) {
    initialize();
    cpu.setup(tokenizer);
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

    // TODO: implement
    tokenizer.skip_until(TokenGroup::newline, true);
}

void Assembler::parse_label(const Token& name) {
    // TODO: implement

}

void Assembler::parse_assignment(const Token &name) {
    // TODO: implement
    tokenizer.skip_until(TokenGroup::newline, true);
}
