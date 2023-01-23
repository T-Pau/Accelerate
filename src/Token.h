/*
Token.h -- Parsing Token

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

#ifndef TOKEN_H
#define TOKEN_H

#include <unordered_set>

#include "Location.h"
#include "SymbolTable.h"

class Token {
public:
    enum Type {
        DIRECTIVE,
        PUNCTUATION,
        NAME,
        END,
        NEWLINE,
        INTEGER,
        PREPROCESSOR,
        REAL,
        STRING,
        KEYWORD,
        INSTRUCTION
    };

    Token(): type(END) {}
    Token(Type type, Location location): type(type), location(location) {}
    Token(Type type, Location location, const std::string& name): Token(type, location) {value.symbol = SymbolTable::global.add(name);}
    Token(Type type, Location location, symbol_t symbol): Token(type, location) {value.symbol = symbol;}
    Token(Type type, Location location, uint64_t integer): Token(type, location) {value.integer = integer;}
    Token(Type type, Location location, double real): Token(type, location) {value.real = real;}

    explicit operator bool() const {return type != END;}
    bool operator==(const Token& other) const;
    bool operator!=(const Token& other) const {return !((*this)==other);}
    [[nodiscard]] Type get_type() const {return type;}
    [[nodiscard]] bool is_directive() const {return type == DIRECTIVE;}
    [[nodiscard]] bool is_integer() const {return type == INTEGER;}
    [[nodiscard]] bool is_name() const {return type == NAME;}
    [[nodiscard]] bool is_newline() const {return type == NEWLINE;}
    [[nodiscard]] bool is_preprocessor() const {return type == PREPROCESSOR;}
    [[nodiscard]] bool is_punctuation() const {return type == PUNCTUATION;}
    [[nodiscard]] bool is_string() const {return type == STRING;}
    [[nodiscard]] const char* type_name() const {return type_name(type);}

    [[nodiscard]] const std::string& as_string() const;
    [[nodiscard]] symbol_t as_symbol() const;
    [[nodiscard]] uint64_t as_integer() const {return type == INTEGER ? value.integer : 0;}
    [[nodiscard]] double as_real() const {return type == REAL ? value.real : 0.0;}

    static const char* type_name(Type type);

    Location location;

private:
    Type type;
    union {
        uint64_t integer;
        double real;
        symbol_t symbol;
    } value = {0};

    static const std::string empty_string;
};

template<>
struct std::hash<Token>
{
    std::size_t operator()(Token const& token) const noexcept {
        size_t h1 = std::hash<size_t>{}(static_cast<size_t>(token.get_type()));
        size_t h2 = 0;
        switch (token.get_type()) {
            case Token::DIRECTIVE:
            case Token::INSTRUCTION:
            case Token::KEYWORD:
            case Token::NAME:
            case Token::PREPROCESSOR:
            case Token::PUNCTUATION:
            case Token::STRING:
                h2 = std::hash<symbol_t>{}(token.as_symbol());
                break;

            case Token::INTEGER:
                h2 = std::hash<uint64_t>{}(token.as_integer());
                break;

            case Token::REAL:
                h2 = std::hash<double>{}(token.as_real());
                break;

            case Token::END:
            case Token::NEWLINE:
                break;
        }
        return h1 ^ (h2 << 1);
    }
};

#endif // TOKEN_H
