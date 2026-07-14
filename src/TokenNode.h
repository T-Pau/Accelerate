#ifdef IN_XLR8_TOKEN_NODE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_TOKEN_NODE_H
#ifndef HAD_XLR8_TOKEN_NODE_H
#define HAD_XLR8_TOKEN_NODE_H

/*
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

#include "Node.h"
#include "Token.h"

/**
 * @brief Represents a token corresponding to punctuation or keyword in the instruction notation.
 */
class TokenNode: public Node {
public:
    /**
     * @brief Initialize a TokenNode with a specific token.
     *
     * @param token The token corresponding to the punctuation or keyword in the instruction notation.
     */
    explicit TokenNode(const Token& token);

    [[nodiscard]] Type type() const override {return node_type;}
    [[nodiscard]] const Location& get_location() const override {return token.location;}

    /**
     * @brief Get the symbol corresponding to the token.
     *
     * @return The symbol corresponding to the token.
     */
    [[nodiscard]] Symbol as_symbol() const {return token.as_symbol();}

private:
    /// @brief The type of the node, which can be KEYWORD or PUNCTUATION.
    Type node_type;

    /// @brief The token corresponding to the punctuation or keyword in the instruction notation.
    Token token;
};


#endif // HAD_XLR8_TOKEN_NODE_H
#undef IN_XLR8_TOKEN_NODE_H
