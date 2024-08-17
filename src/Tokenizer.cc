/*
Tokenizer.cc -- 

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

#include "Tokenizer.h"
#include "Exception.h"
#include "ParseException.h"

Token Tokenizer::next() {
    if (ungot_token.has_value()) {
        auto token = ungot_token.value();
        ungot_token.reset();
        return token;
    }

    return sub_next();
}



void Tokenizer::unget(const Token& token) {
    if (ungot_token.has_value()) {
        throw Exception("trying to unget two tokens");
    }
    if (!token) {
        return;
    }
    ungot_token = token;
}

Token Tokenizer::expect(Token::Type type) {
    auto token = next();
    if (token.get_type() == type) {
        return token;
    }
    else {
        throw ParseException(token.location, "expected %s, got %s", Token::type_name(type), token.type_name());
    }
}

Token Tokenizer::expect(Token::Type type, const TokenGroup& synchronize) {
    auto token = next();
    if (token.get_type() == type) {
        return token;
    }
    else {
        skip_until(synchronize);
        throw ParseException(token.location, "expected %s, got %s", Token::type_name(type), token.type_name());
    }
}

Token Tokenizer::expect(const TokenGroup& types, const TokenGroup& synchronize) {
    auto token = next();
    if (types.contains(token)) {
        return token;
    }
    else {
        skip_until(synchronize);
        throw ParseException(token.location, "expected %s, got %s", types.name.c_str(), token.type_name());
    }
}


void Tokenizer::skip_until(const TokenGroup &types, bool including_terminator) {
    Token token;
    while ((token = next())) {
        if (types.contains(token)) {
            if (!including_terminator) {
                unget(token);
            }
            return;
        }
    }
}


void Tokenizer::skip_until(const Token& token, bool including_terminator) {
    Token next_token;
    while ((next_token = next())) {
        if (next_token == token) {
            if (!including_terminator) {
                unget(next_token);
            }
            return;
        }
    }
}

void Tokenizer::expect_litearls(const std::vector<Token::Type>& types, const TokenGroup &synchronize) {
    for (auto type : types) {
        expect(type, synchronize);
    }
}

void Tokenizer::skip(const TokenGroup &types) {
    Token token;
    while ((token = next())) {
        if (!types.contains(token)) {
            unget(token);
            return;
        }
    }
}


void Tokenizer::skip(Token::Type type) {
    Token token;
    while ((token = next())) {
        if (token.get_type() != type) {
            unget(token);
            return;
        }
    }
}


std::vector<Token> Tokenizer::collect_until(Token::Type type) {
    std::vector<Token> tokens;

    Token token;
    while ((token = next())) {
        if (token.get_type() != type) {
            tokens.emplace_back(token);
        }
        else {
            break;
        }
    }
    unget(token);
    return tokens;
}


std::vector<Token> Tokenizer::collect_until(const TokenGroup& types) {
    std::vector<Token> tokens;

    Token token;
    while ((token = next())) {
        if (!types.contains(token)) {
            tokens.emplace_back(token);
        }
    }
    unget(token);
    return tokens;
}

Token Tokenizer::peek() {
    auto token = next();
    unget(token);
    return token;
}

void Tokenizer::expect(const Token& token) {
    auto got = next();
    if (got != token) {
        throw ParseException(got, "expected %s", token.as_string().c_str());
    }
}
