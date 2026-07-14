#ifdef IN_XLR8_STRUCTURED_SCALAR_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_STRUCTURED_SCALAR_H
#ifndef HAD_XLR8_STRUCTURED_SCALAR_H
#define HAD_XLR8_STRUCTURED_SCALAR_H

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

/**
 * @brief Represents a scalar value, which can be a single token or a list of tokens.
 */
class StructuredScalar: public StructuredValue {
public:
    explicit StructuredScalar(Tokenizer& tokenizer);

    std::vector<Token> tokens;

    [[nodiscard]] bool empty() const {return tokens.empty();}
    [[nodiscard]] size_t size() const {return tokens.size();}
    const Token& operator[](size_t index) const {return tokens[index];}
    [[nodiscard]] const Token& token() const {return tokens.front();}
    [[nodiscard]] std::vector<Token>::iterator begin() {return tokens.begin();}
    [[nodiscard]] std::vector<Token>::iterator end() {return tokens.end();}
    [[nodiscard]] std::vector<Token>::const_iterator begin() const {return tokens.begin();}
    [[nodiscard]] std::vector<Token>::const_iterator end() const {return tokens.end();}

    [[nodiscard]] Type type() const override {return tokens.size() == 1 ? SCALAR_SINGULAR : SCALAR_LIST;}
};


#endif // HAD_XLR8_STRUCTURED_SCALAR_H
#undef IN_XLR8_STRUCTURED_SCALAR_H
