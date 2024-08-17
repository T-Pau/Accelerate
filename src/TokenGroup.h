/*
TokenGroup.h -- 

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

#ifndef TOKEN_GROUP_H
#define TOKEN_GROUP_H

#include "Token.h"

class TokenGroup {
public:
    explicit TokenGroup(bool contains_all = false): contains_all{contains_all}, name{contains_all ? "all": "none"} {}
    explicit TokenGroup(Token::Type type): types({ type }), name (Token::type_name(type)) {}
    TokenGroup(std::unordered_set<Token::Type> types, std::unordered_set<Token> tokens, std::string name): types (std::move(types)), tokens(std::move(tokens)), name(std::move(name)) {}

    [[nodiscard]] bool contains(const Token& token) const {return contains_all || types.contains(token.get_type()) || tokens.contains(token);}

    static const TokenGroup newline;
    static const TokenGroup all;

    std::unordered_set<Token::Type> types;
    std::unordered_set<Token> tokens;
    bool contains_all{false};
    std::string name;
};


#endif // TOKEN_GROUP_H
