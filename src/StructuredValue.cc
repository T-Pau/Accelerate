/*
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

#include "StructuredValue.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "BodyParser.h"
#include "Scope.h"
#include "StructuredArray.h"
#include "StructuredBody.h"
#include "StructuredDictionary.h"
#include "StructuredScalar.h"

using namespace tpau::cpp_kernal;

bool StructuredValue::initialized = false;
TokenGroup StructuredValue::start_group;

void StructuredValue::initialize() {
    if (!initialized) {

        start_group = TokenGroup({}, {Token::less, Token::colon, Token::curly_open, Token::square_open}, "object start");

        initialized = true;
    }
}


std::shared_ptr<StructuredValue> StructuredValue::parse(Tokenizer &tokenizer) {
    initialize();

    auto token = tokenizer.expect(start_group, TokenGroup(Token::NEWLINE));

    std::shared_ptr<StructuredValue> object;

    if (token == Token::less) {
        object = std::make_shared<StructuredBody>(tokenizer);
    }
    else if (token == Token::colon) {
        object = std::make_shared<StructuredScalar>(tokenizer);
    }
    else if (token == Token::curly_open) {
        object = std::make_shared<StructuredDictionary>(tokenizer);
    }
    else if (token == Token::square_open) {
        object = std::make_shared<StructuredArray>(tokenizer);
    }
    else {
        throw LocationException(token.location, "unexpected {}", token.type_name());
    }

    object->location = token.location;
    //object->location.extend(tokenizer.current_location());
    return object;
}

void StructuredValue::setup(FileTokenizer &tokenizer) {
    tokenizer.add_punctuations({"{", "}", "[", "]", ":"});
}


const StructuredArray *StructuredValue::as_array() const {
    if (!is_array()) {
        throw LocationException(location, "array expected");
    }

    return reinterpret_cast<const StructuredArray*>(this);
}


const StructuredBody *StructuredValue::as_body() const {
    if (!is_body()) {
        throw LocationException(location, "body expected");
    }

    return reinterpret_cast<const StructuredBody*>(this);
}


const StructuredDictionary *StructuredValue::as_dictionary() const {
    if (!is_dictionary()) {
        throw LocationException(location, "dictionary expected");
    }

    return reinterpret_cast<const StructuredDictionary*>(this);
}


const StructuredScalar *StructuredValue::as_scalar() const {
    if (!is_scalar()) {
        throw LocationException(location, "scalar expected");
    }

    return reinterpret_cast<const StructuredScalar*>(this);
}


const StructuredScalar *StructuredValue::as_singular_scalar() const {
    if (!is_singular_scalar()) {
        throw LocationException(location, "singular scalar expected");
    }

    return reinterpret_cast<const StructuredScalar*>(this);
}


StructuredArray::StructuredArray(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    while (true) {
        auto token = tokenizer.next();

        if (token == Token::square_close) {
            break;
        }

        tokenizer.unget(token);
        entries.emplace_back(StructuredValue::parse(tokenizer));
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}


StructuredDictionary::StructuredDictionary(Tokenizer &tokenizer) {
    tokenizer.skip(Token::NEWLINE);
    while (true) {
        auto token = tokenizer.next();
        if (token == Token::curly_close) {
            break;
        }

        entries[token] = StructuredValue::parse(tokenizer);
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}

std::shared_ptr<StructuredValue> StructuredDictionary::get_optional(const Token &token) const {
    auto it = entries.find(token);
    if (it == entries.end()) {
        return {};
    }

    return it->second;
}



std::shared_ptr<StructuredValue> StructuredDictionary::operator[](const Token& token) const {
    auto value = get_optional(token);
    if (value == nullptr) {
            throw LocationException(location, "missing key '{}'", token);
    }

    return value;
}


StructuredScalar::StructuredScalar(Tokenizer &tokenizer) {
    tokens = tokenizer.collect_until(Token::NEWLINE);
    tokenizer.skip(Token::NEWLINE);
}


StructuredBody::StructuredBody(Tokenizer &tokenizer) {
    // TODO: this shouldn't need an environment
    auto parser = BodyParser(tokenizer, std::make_shared<Scope>(Visibility::SCOPE));

    body = parser.parse();
    tokenizer.skip(Token::NEWLINE);
}
