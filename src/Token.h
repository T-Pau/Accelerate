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
#include "Symbol.h"
#include "Value.h"

class Token {
public:
    enum Type {
        DIRECTIVE,
        PUNCTUATION,
        NAME,
        END,
        NEWLINE,
        PREPROCESSOR,
        STRING,
        KEYWORD,
        INSTRUCTION,
        VALUE
    };

    Token(): type(END) {}
    Token(Type type, Location location): type(type), location(location) {}
    Token(Type type, const std::string& name): Token(type, Location()) {symbol = Symbol(name);}
    Token(Type type, Location location, Symbol symbol_): Token(type, location) {symbol = symbol_;}
    Token(Type type, Location location, const std::string& string): Token(type, location, Symbol(string)) {}
    Token(Type type, Location location, uint64_t integer, uint64_t default_size = 0): Token(type, location) {value = Value(integer, default_size);}
    Token(Type type, Location location, double real): Token(type, location) {value = Value(real);}
//    Token(const Token& other): type(other.type) {*this = other;}

//    Token& operator=(const Token& other);

    explicit operator bool() const {return type != END;}
    bool operator==(const Token& other) const;
    bool operator!=(const Token& other) const {return !((*this)==other);}
    [[nodiscard]] Type get_type() const {return type;}
    [[nodiscard]] bool is_directive() const {return type == DIRECTIVE;}
    [[nodiscard]] bool is_value() const {return type == VALUE;}
    [[nodiscard]] bool is_integer() const {return is_value() && value.is_integer();}
    [[nodiscard]] bool is_unsigned() const {return is_value() && value.is_unsigned();}
    [[nodiscard]] bool is_name() const {return type == NAME;}
    [[nodiscard]] bool is_newline() const {return type == NEWLINE;}
    [[nodiscard]] bool is_preprocessor() const {return type == PREPROCESSOR;}
    [[nodiscard]] bool is_punctuation() const {return type == PUNCTUATION;}
    [[nodiscard]] bool is_string() const {return type == STRING;}
    [[nodiscard]] const char* type_name() const {return type_name(type);}

    [[nodiscard]] const std::string& as_string() const;
    [[nodiscard]] Symbol as_symbol() const;
    [[nodiscard]] uint64_t as_unsigned() const {return is_unsigned() ? value.unsigned_value() : 0;}
    [[nodiscard]] Value as_value() const {return is_value() ? value : Value();}

    static const char* type_name(Type type);

    Location location;

    static const Token colon;
    static const Token comma;
    static const Token curly_close;
    static const Token curly_open;
    static const Token equals;
    static const Token greater;
    static const Token less;
    static const Token paren_close;
    static const Token paren_open;

private:
    Type type;
    union {
        Value value;
        Symbol symbol;
    };

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
                h2 = std::hash<Symbol>{}(token.as_symbol());
                break;

            case Token::VALUE:
                h2 = std::hash<Value>{}(token.as_value());
                break;

            case Token::END:
            case Token::NEWLINE:
                break;
        }
        return h1 ^ (h2 << 1);
    }
};

#endif // TOKEN_H
