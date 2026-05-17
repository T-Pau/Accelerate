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

/// @brief A group of tokens specified by specific tokens or token types.
class TokenGroup {
public:
    /**
     * Create a token group that contains no or all tokens.
     * 
     * @param contains_all If `true`, the group contains all tokens, otherwise it contains no tokens.
     */
    explicit TokenGroup(bool contains_all = false): contains_all{contains_all}, name{contains_all ? "all": "none"} {}

    /**
     * Create a token group that contains all tokens of the given type.
     * 
     * @param type The type of tokens to include in the group.
     */
    explicit TokenGroup(Token::Type type): types({ type }), name (Token::type_name(type)) {}

    /**
     * Create a token group that contains the given tokens.
     * 
     * @param tokens The tokens to include in the group.
     * @param name The name of the token group.
     */
    TokenGroup(std::unordered_set<Token::Type> types, std::unordered_set<Token> tokens, std::string name): types (std::move(types)), tokens(std::move(tokens)), name(std::move(name)) {}

    /**
     * Check if the group contains a given token.
     * 
     * @param token The token to check.
     * @return `true` if the group contains the token, `false` otherwise.
     */
    [[nodiscard]] bool contains(const Token& token) const {return contains_all || types.contains(token.get_type()) || tokens.contains(token);}

    /// @brief A token group that contains the newline token.
    static const TokenGroup newline;

    /// @brief A token group that contains all tokens.
    static const TokenGroup all;

    /// @brief The token types contained in the group.
    std::unordered_set<Token::Type> types;

    /// @brief The specific tokens contained in the group.
    std::unordered_set<Token> tokens;

    /// @brief Whether the group contains all tokens.
    bool contains_all{false};

    /// @brief The name of the token group.
    std::string name;
};


#endif // TOKEN_GROUP_H
