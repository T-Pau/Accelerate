/*
Tokenizer.h -- 

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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <optional>

#include "Token.h"
#include "TokenGroup.h"

class Tokenizer {
public:
    Token next();
    void unget(Token token);
    Token peek();
    virtual ~Tokenizer() = default;

    std::vector<Token> collect_until(Token::Type type);
    std::vector<Token> collect_until(const TokenGroup& types);
    Token expect(Token::Type type);
    Token expect(Token::Type type, const TokenGroup& synchronize);
    Token expect(const TokenGroup& types, const TokenGroup& synchronize);
    void expect(Token token);
    void expect_litearls(const std::vector<Token::Type>& types, const TokenGroup& synchronize);
    void skip_until(const TokenGroup& types, bool including_terminator = false);
    void skip_until(const Token& token, bool including_terminator = false);
    void skip(const TokenGroup& types);
    void skip(Token::Type type);

    [[nodiscard]] virtual Location current_location() const = 0;
    [[nodiscard]] bool ended() const {return !ungot_token.has_value() && sub_ended();}

protected:
    virtual Token sub_next() = 0;
    [[nodiscard]] virtual bool sub_ended() const = 0;

private:
    std::optional<Token> ungot_token;

};

#endif // TOKENIZER_H
