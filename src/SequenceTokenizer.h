/*
SequenceTokenizer.h --

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

#ifndef SEQUENCE_TOKENIZER_H
#define SEQUENCE_TOKENIZER_H

#include <vector>

#include "Tokenizer.h"

class SequenceTokenizer: public Tokenizer {
public:
    SequenceTokenizer(const Location& location, std::vector<Token> tokens): location(location), tokens(std::move(tokens)) { current_position = this->tokens.begin();}
    explicit SequenceTokenizer(std::vector<Token> tokens);

    [[nodiscard]] Location current_location() const override {return location;}

protected:
    [[nodiscard]] bool sub_ended() const override {return current_position == tokens.end();}
    Token sub_next() override;

private:
    Location location;
    std::vector<Token> tokens;
    std::vector<Token>::iterator current_position;

    void set_location(const Token& token);
};

#endif // SEQUENCE_TOKENIZER_H
