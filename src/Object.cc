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

const Token::Group Object::start_group({Token::COLON, Token::CURLY_PARENTHESIS_OPEN, Token::SQUARE_PARENTHESIS_OPEN}, "object start");

std::shared_ptr<Object> Object::parse(Tokenizer &tokenizer) {
    auto token = tokenizer.expect(start_group, Token::Group(Token::NEWLINE));

    switch (token.get_type()) {
        case Token::COLON:
            return std::make_shared<ObjectScalar>(tokenizer);
        case Token::CURLY_PARENTHESIS_OPEN:
            return std::make_shared<ObjectDictionary>(tokenizer);
        case Token::SQUARE_PARENTHESIS_OPEN:
            return std::make_shared<ObjectArray>(tokenizer);

        default:
            throw ParseException(token, "unexpected %s", token.type_name());
    }
}


ObjectArray::ObjectArray(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    auto done = false;
    while (!done) {
        auto token = tokenizer.next();
        switch (token.get_type()) {
            case Token::END:
            case Token::CURLY_PARENTHESIS_CLOSE:
                throw ParseException(token, "unexpected %s", token.type_name());

            case Token::SQUARE_PARENTHESIS_CLOSE:
                done = true;
                break;

            default:
                tokenizer.unget(token);
                entries.emplace_back(Object::parse(tokenizer));
        }
    }
    tokenizer.expect(Token::NEWLINE, Token::Group(Token::NEWLINE));
}

ObjectDictionary::ObjectDictionary(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    auto done = false;
    while (!done) {
        auto token = tokenizer.next();
        switch (token.get_type()) {
            case Token::END:
            case Token::SQUARE_PARENTHESIS_CLOSE:
                throw ParseException(token, "unexpected %s", token.type_name());

            case Token::CURLY_PARENTHESIS_CLOSE:
                done = true;
                break;

            default:
                entries[token] = Object::parse(tokenizer);
        }
    }
    tokenizer.expect(Token::NEWLINE, Token::Group(Token::NEWLINE));
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
