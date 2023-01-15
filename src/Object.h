/*
Object.h -- 

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

#ifndef OBJECT_H
#define OBJECT_H

#include <unordered_map>
#include <vector>

#include "Token.h"
#include "Tokenizer.h"
#include "TokenGroup.h"

class ObjectArray;
class ObjectDictionary;
class ObjectScalar;

class Object {
public:
    enum Type {
        ARRAY,
        DICTIONARY,
        SCALAR_LIST,
        SCALAR_SINGULAR
    };

    [[nodiscard]] virtual Type type() const = 0;

    static void setup(Tokenizer& tokenizer);
    static std::shared_ptr<Object> parse(Tokenizer& tokenizer);

    [[nodiscard]] const ObjectArray* as_array() const {return is_array() ? reinterpret_cast<const ObjectArray*>(this) : nullptr;}
    [[nodiscard]] const ObjectDictionary* as_dictionary() const {return is_dictionary() ? reinterpret_cast<const ObjectDictionary*>(this) : nullptr;}
    [[nodiscard]] const ObjectScalar* as_scalar() const {return is_scalar() ? reinterpret_cast<const ObjectScalar*>(this) : nullptr;}
    [[nodiscard]] bool is_array() const {return type() == ARRAY;}
    [[nodiscard]] bool is_dictionary() const {return type() == DICTIONARY;}
    [[nodiscard]] bool is_scalar() const {return type() == SCALAR_SINGULAR || type() == SCALAR_LIST;}
    [[nodiscard]] bool is_singular_scalar() const {return type() == SCALAR_SINGULAR;}

    Location location;

protected:
    static void initialize();
    static bool initialized;
    static TokenGroup start_group;
    static Token token_colon;
    static Token token_curly_close;
    static Token token_curly_open;
    static Token token_square_close;
    static Token token_square_open;
};


class ObjectArray: public Object {
public:
    explicit ObjectArray(Tokenizer& tokenizer);

    std::vector<std::shared_ptr<Object>> entries;

    [[nodiscard]] Type type() const override {return ARRAY;}

    [[nodiscard]] bool empty() const {return entries.empty();}
    [[nodiscard]] size_t size() const {return entries.size();}
    std::shared_ptr<Object> operator[](size_t index) const {return entries[index];}

    std::vector<std::shared_ptr<Object>>::iterator begin() {return entries.begin();}
    std::vector<std::shared_ptr<Object>>::iterator end() {return entries.end();}
    [[nodiscard]] std::vector<std::shared_ptr<Object>>::const_iterator begin() const {return entries.begin();}
    [[nodiscard]] std::vector<std::shared_ptr<Object>>::const_iterator end() const {return entries.end();}
};


class ObjectDictionary: public Object {
public:
    explicit ObjectDictionary(Tokenizer& tokenizer);

    std::unordered_map<Token, std::shared_ptr<Object>> entries;

    [[nodiscard]] Type type() const override {return DICTIONARY;}

    std::shared_ptr<Object> operator[](const Token& token) const;
    std::shared_ptr<Object> operator[](const std::string& name) const {return (*this)[Token(Token::NAME, {}, SymbolTable::global.add(name))];}

    std::unordered_map<Token, std::shared_ptr<Object>>::iterator begin() {return entries.begin();}
    std::unordered_map<Token, std::shared_ptr<Object>>::iterator end() {return entries.end();}
    [[nodiscard]] std::unordered_map<Token, std::shared_ptr<Object>>::const_iterator begin() const {return entries.begin();}
    [[nodiscard]] std::unordered_map<Token, std::shared_ptr<Object>>::const_iterator end() const {return entries.end();}
};


class ObjectScalar: public Object {
public:
    explicit ObjectScalar(Tokenizer& tokenizer);

    std::vector<Token> tokens;

    [[nodiscard]] bool empty() const {return tokens.empty();}
    [[nodiscard]] size_t size() const {return tokens.size();}
    const Token& operator[](size_t index) const {return tokens[index];}
    [[nodiscard]] const Token& token() const {return tokens.front();}
    [[nodiscard]] std::vector<Token>::iterator begin() {return tokens.begin();}
    [[nodiscard]] std::vector<Token>::iterator end() {return tokens.end();}
    [[nodiscard]] std::vector<Token>::const_iterator begin() const {return tokens.begin();}
    [[nodiscard]] std::vector<Token>::const_iterator end() const {return tokens.end();}

    [[nodiscard]] Type type() const override {return tokens.size() == 1 ? SCALAR_SINGULAR : SCALAR_LIST;}
};


#endif // OBJECT_H
