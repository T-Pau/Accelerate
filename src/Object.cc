/*
Object.cc -- 

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

#include "Object.h"
#include "ParseException.h"

bool Object::initialized = false;
TokenGroup Object::start_group;
Token Object::token_colon;
Token Object::token_curly_close;
Token Object::token_curly_open;
Token Object::token_square_close;
Token Object::token_square_open;

void Object::initialize() {
    if (!initialized) {
        token_colon = Token(Token::PUNCTUATION, {}, ":");
        token_curly_close = Token(Token::PUNCTUATION, {}, "}");
        token_curly_open = Token(Token::PUNCTUATION, {}, "{");
        token_square_close = Token(Token::PUNCTUATION, {}, "]");
        token_square_open = Token(Token::PUNCTUATION, {}, "[");

        start_group = TokenGroup({}, {token_colon, token_curly_open, token_square_open}, "object start");

        initialized = true;
    }
}


std::shared_ptr<Object> Object::parse(Tokenizer &tokenizer) {
    initialize();

    auto token = tokenizer.expect(start_group, TokenGroup(Token::NEWLINE));

    std::shared_ptr<Object> object;

    if (token == token_colon) {
        object = std::make_shared<ObjectScalar>(tokenizer);
    }
    else if (token == token_curly_open) {
        object = std::make_shared<ObjectDictionary>(tokenizer);
    }
    else if (token == token_square_open) {
        object = std::make_shared<ObjectArray>(tokenizer);
    }
    else {
        throw ParseException(token, "unexpected %s", token.type_name());
    }

    object->location = token.location;
    object->location.extend(tokenizer.current_location());
    return object;
}

void Object::setup(Tokenizer &tokenizer) {
    tokenizer.add_punctuations({"{", "}", "[", "]", ":"});
}


ObjectArray::ObjectArray(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    while (true) {
        auto token = tokenizer.next();

        if (token == token_square_close) {
            break;
        }

        tokenizer.unget(token);
        entries.emplace_back(Object::parse(tokenizer));
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}


ObjectDictionary::ObjectDictionary(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    while (true) {
        auto token = tokenizer.next();
        if (token == token_curly_close) {
            break;
        }

        entries[token] = Object::parse(tokenizer);
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}


std::shared_ptr<Object> ObjectDictionary::operator[](const Token& token) const {
    auto it = entries.find(token);
    if (it != entries.end()) {
        return it->second;
    }
    else {
        return {};
    }
}


ObjectScalar::ObjectScalar(Tokenizer &tokenizer) {
    tokens = tokenizer.collect_until(Token::NEWLINE);
    tokenizer.skip(Token::NEWLINE);
}
