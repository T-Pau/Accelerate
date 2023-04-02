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

bool ParsedValue::initialized = false;
TokenGroup ParsedValue::start_group;
Token ParsedValue::token_colon;
Token ParsedValue::token_curly_close;
Token ParsedValue::token_curly_open;
Token ParsedValue::token_square_close;
Token ParsedValue::token_square_open;

void ParsedValue::initialize() {
    if (!initialized) {
        token_colon = Token(Token::PUNCTUATION, ":");
        token_curly_close = Token(Token::PUNCTUATION, "}");
        token_curly_open = Token(Token::PUNCTUATION, "{");
        token_square_close = Token(Token::PUNCTUATION, "]");
        token_square_open = Token(Token::PUNCTUATION, "[");

        start_group = TokenGroup({}, {token_colon, token_curly_open, token_square_open}, "object start");

        initialized = true;
    }
}


std::shared_ptr<ParsedValue> ParsedValue::parse(Tokenizer &tokenizer) {
    initialize();

    auto token = tokenizer.expect(start_group, TokenGroup(Token::NEWLINE));

    std::shared_ptr<ParsedValue> object;

    if (token == token_colon) {
        object = std::make_shared<ParsedScalar>(tokenizer);
    }
    else if (token == token_curly_open) {
        object = std::make_shared<ParsedDictionary>(tokenizer);
    }
    else if (token == token_square_open) {
        object = std::make_shared<ParsedArray>(tokenizer);
    }
    else {
        throw ParseException(token, "unexpected %s", token.type_name());
    }

    object->location = token.location;
    //object->location.extend(tokenizer.current_location());
    return object;
}

void ParsedValue::setup(TokenizerFile &tokenizer) {
    tokenizer.add_punctuations({"{", "}", "[", "]", ":"});
}


const ParsedArray *ParsedValue::as_array() const {
    if (!is_array()) {
        throw ParseException(location, "array expected");
    }

    return reinterpret_cast<const ParsedArray*>(this);
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

        if (token == token_square_close) {
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
        if (token == token_curly_close) {
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
