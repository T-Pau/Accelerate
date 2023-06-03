/*
ParsedValue.cc --

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

#include "ParsedValue.h"
#include "ParseException.h"
#include "BodyParser.h"

bool ParsedValue::initialized = false;
TokenGroup ParsedValue::start_group;

void ParsedValue::initialize() {
    if (!initialized) {

        start_group = TokenGroup({}, {Token::less, Token::colon, Token::curly_open, Token::square_open}, "object start");

        initialized = true;
    }
}


std::shared_ptr<ParsedValue> ParsedValue::parse(Tokenizer &tokenizer) {
    initialize();

    auto token = tokenizer.expect(start_group, TokenGroup(Token::NEWLINE));

    std::shared_ptr<ParsedValue> object;

    if (token == Token::less) {
        object = std::make_shared<ParsedBody>(tokenizer);
    }
    else if (token == Token::colon) {
        object = std::make_shared<ParsedScalar>(tokenizer);
    }
    else if (token == Token::curly_open) {
        object = std::make_shared<ParsedDictionary>(tokenizer);
    }
    else if (token == Token::square_open) {
        object = std::make_shared<ParsedArray>(tokenizer);
    }
    else {
        throw ParseException(token, "unexpected %s", token.type_name());
    }

    object->location = token.location;
    //object->location.extend(tokenizer.current_location());
    return object;
}

void ParsedValue::setup(FileTokenizer &tokenizer) {
    tokenizer.add_punctuations({"{", "}", "[", "]", ":"});
}


const ParsedArray *ParsedValue::as_array() const {
    if (!is_array()) {
        throw ParseException(location, "array expected");
    }

    return reinterpret_cast<const ParsedArray*>(this);
}


const ParsedBody *ParsedValue::as_body() const {
    if (!is_body()) {
        throw ParseException(location, "body expected");
    }

    return reinterpret_cast<const ParsedBody*>(this);
}


const ParsedDictionary *ParsedValue::as_dictionary() const {
    if (!is_dictionary()) {
        throw ParseException(location, "dictionary expected");
    }

    return reinterpret_cast<const ParsedDictionary*>(this);
}


const ParsedScalar *ParsedValue::as_scalar() const {
    if (!is_scalar()) {
        throw ParseException(location, "scalar expected");
    }

    return reinterpret_cast<const ParsedScalar*>(this);
}


const ParsedScalar *ParsedValue::as_singular_scalar() const {
    if (!is_singular_scalar()) {
        throw ParseException(location, "singular scalar expected");
    }

    return reinterpret_cast<const ParsedScalar*>(this);
}


ParsedArray::ParsedArray(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    while (true) {
        auto token = tokenizer.next();

        if (token == Token::square_close) {
            break;
        }

        tokenizer.unget(token);
        entries.emplace_back(ParsedValue::parse(tokenizer));
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}


ParsedDictionary::ParsedDictionary(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    while (true) {
        auto token = tokenizer.next();
        if (token == Token::curly_close) {
            break;
        }

        entries[token] = ParsedValue::parse(tokenizer);
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}

std::shared_ptr<ParsedValue> ParsedDictionary::get_optional(const Token &token) const {
    auto it = entries.find(token);
    if (it == entries.end()) {
        return {};
    }

    return it->second;
}



std::shared_ptr<ParsedValue> ParsedDictionary::operator[](const Token& token) const {
    auto value = get_optional(token);
    if (value == nullptr) {
            throw ParseException(location, "missing key '%s'", token.as_string().c_str());
    }

    return value;
}


ParsedScalar::ParsedScalar(Tokenizer &tokenizer) {
    tokens = tokenizer.collect_until(Token::NEWLINE);
    tokenizer.skip(Token::NEWLINE);
}


ParsedBody::ParsedBody(Tokenizer &tokenizer) {
    auto parser = BodyParser(tokenizer);

    body = parser.parse();
    tokenizer.skip(Token::NEWLINE);
}
