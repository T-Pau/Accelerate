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

#include "Visibility.h"

// Keep in sync with Visibility.
Symbol VisibilityHelper::names[] = {
    "scope",
    "entity",
    "file",
    Symbol(VisibilityHelper::private_literal),
    Symbol(VisibilityHelper::public_literal)
};

Symbol VisibilityHelper::file_literal = "local";
Symbol VisibilityHelper::private_literal = "private";
Symbol VisibilityHelper::public_literal = "public";

const Token VisibilityHelper::token_file_directive = Token(Token::DIRECTIVE, {}, VisibilityHelper::file_literal);
const Token VisibilityHelper::token_file_name = Token(Token::NAME, {}, VisibilityHelper::file_literal);
const Token VisibilityHelper::token_private_directive = Token(Token::DIRECTIVE, {}, VisibilityHelper::private_literal);
const Token VisibilityHelper::token_private_name = Token(Token::NAME, {}, VisibilityHelper::private_literal);
const Token VisibilityHelper::token_public_directive = Token(Token::DIRECTIVE, {}, VisibilityHelper::public_literal);
const Token VisibilityHelper::token_public_name = Token(Token::NAME, {}, VisibilityHelper::public_literal);

std::ostream& operator<<(std::ostream& stream, Visibility visibility) {
    if (visibility == Visibility::FILE) {
        stream << VisibilityHelper::file_literal;
    }
    else {
        stream << VisibilityHelper::name(visibility);
    }
    return stream;
}

std::optional<Visibility> VisibilityHelper::from_token(const Token& token) {
    if (token == token_file_directive || token == token_file_name) {
        return Visibility::FILE;
    }
    else if (token == token_private_directive || token == token_private_name) {
        return Visibility::PRIVATE;
    }
    else if (token == token_public_directive || token == token_public_name) {
        return Visibility::PUBLIC;
    }
    else {
        return {};
    }
}
