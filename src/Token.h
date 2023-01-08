/*
Token.h -- Parsing Token

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

#include <unordered_set>

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

    class Group {
    public:
        explicit Group(Type type): types({type}), name(type_name(type)) {}
        explicit Group(std::unordered_set<Type> types, std::string name): types(std::move(types)), name(std::move(name)) {}

        [[nodiscard]] bool contains(Type type) const {return types.find(type) != types.end();}

        std::unordered_set<Type> types;
        std::string name;
    };

    Token(): type(END) {}
    explicit Token(Type type): type(type) {}
    Token(Type type, Location location): type(type), location(std::move(location)) {}
    Token(Type type, Location location, std::string name): type(type), location(std::move(location)), name(std::move(name)) {}
    Token(Type type, Location location, uint64_t integer): Token(type, std::move(location)) { value.integer = integer; }
    Token(Type type, Location location, double real): Token(type, std::move(location)) { value.real = real; }

    explicit operator bool() const {return type != END;}
    bool operator==(const Token& other) const;
    [[nodiscard]] bool is_directive() const {return type == DIRECTIVE;}
    [[nodiscard]] bool is_name() const {return type == NAME;}
    [[nodiscard]] bool is_newline() const {return type == NEWLINE;}
    [[nodiscard]] const char* type_name() const {return type_name(type);}

    static const char* type_name(Type type);

    Type type;
    std::string name;
    union {
        uint64_t integer;
        double real;
    } value = {0};
    Location location;
};

template<>
struct std::hash<Token>
{
    std::size_t operator()(Token const& token) const noexcept {
        size_t h1 = std::hash<size_t>{}(static_cast<size_t>(token.type));
        size_t h2 = 0;
        switch (token.type) {
            case Token::NAME:
            case Token::DIRECTIVE:
                h2 = std::hash<std::string>{}(token.name);
                break;

            case Token::NUMBER:
                h2 = std::hash<::uint64_t>{}(token.value.integer);
                break;

            default:
                break;
        }
        return h1 ^ (h2 << 1);
    }
};

#endif // TOKEN_H
