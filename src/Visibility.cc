/*
Visibility.cc --

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

#define PRIVATE_LITERAL "private"
#define PUBLIC_LITERAL "public"

bool VisibilityHelper::initialized = false;
Token VisibilityHelper::token_private_directive;
Token VisibilityHelper::token_private_name;
Token VisibilityHelper::token_public_directive;
Token VisibilityHelper::token_public_name;

std::ostream& operator<<(std::ostream& stream, Visibility visibility) {
    switch  (visibility) {
        case Visibility::SCOPE:
            stream << "none";
            break;
        case Visibility::PUBLIC:
            stream << PUBLIC_LITERAL;
            break;
        case Visibility::PRIVATE:
            stream << PRIVATE_LITERAL;
            break;
    }
    return stream;
}

void VisibilityHelper::initialize() {
    if (!initialized) {
        token_public_directive = Token(Token::DIRECTIVE, PUBLIC_LITERAL);
        token_public_name = Token(Token::NAME, PUBLIC_LITERAL);
        token_private_directive = Token(Token::DIRECTIVE, PRIVATE_LITERAL);
        token_private_name = Token(Token::NAME, PRIVATE_LITERAL);
        initialized = true;
    }
}

void VisibilityHelper::setup(FileTokenizer& tokenizer, bool use_directives) {
    initialize();
}

std::optional<Visibility> VisibilityHelper::from_token(Token token) {
    if (token == token_private_directive || token == token_private_name) {
        return Visibility::PRIVATE;
    }
    else if (token == token_public_directive || token == token_public_name) {
        return Visibility::PUBLIC;
    }
    else {
        return {};
    }
}
