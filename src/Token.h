/*
Token.cc -- Parsing Token

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

#ifndef TOKEN_H
#define TOKEN_H


#include "Location.h"

class Token {
public:
    enum Type {
        COLON,
        COMMA,
        CURLY_PARENTHESIS_CLOSE,
        CURLY_PARENTHESIS_OPEN,
        DIRECTIVE,
        END,
        EQUAL,
        ERROR,
        GREATER,
        HASH,
        LESS,
        MINUS,
        NAME,
        NEWLINE,
        NUMBER,
        PARENTHESIS_CLOSE,
        PARENTHESIS_OPEN,
        PLUS,
        SQUARE_PARENTHESIS_CLOSE,
        SQUARE_PARENTHESIS_OPEN,
        STAR,
        STRING
    };

    explicit Token(Type type): type(type) {}
    Token(Type type, Location location): type(type), location(std::move(location)) {}
    Token(Type type, Location location, std::string name): type(type), location(std::move(location)), name(std::move(name)) {}
    Token(Type type, Location location, uint64_t integer): Token(type, std::move(location)) { value.integer = integer; }
    Token(Type type, Location location, double real): Token(type, std::move(location)) { value.real = real; }

    Type type;
    std::string name;
    union {
        uint64_t integer;
        double real;
    } value = {0};
    Location location;
};

#endif // TOKEN_H
