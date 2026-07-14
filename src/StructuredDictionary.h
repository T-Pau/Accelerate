#ifdef IN_XLR8_STRUCTURED_DICTIONARY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_STRUCTURED_DICTIONARY_H
#ifndef HAD_XLR8_STRUCTURED_DICTIONARY_H
#define HAD_XLR8_STRUCTURED_DICTIONARY_H

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

#include <unordered_map>

#include "StructuredValue.h"

/**
 * @brief Represents a dictionary of structured values. Keys are tokens and values are structured values.
 */
class StructuredDictionary: public StructuredValue {
public:
    explicit StructuredDictionary(Tokenizer& tokenizer);

    std::unordered_map<Token, std::shared_ptr<StructuredValue>> entries;

    [[nodiscard]] Type type() const override {return DICTIONARY;}

    std::shared_ptr<StructuredValue> operator[](const Token& token) const;
    [[nodiscard]] std::shared_ptr<StructuredValue> get_optional(const Token& token) const;
    [[nodiscard]] bool has_key(const Token& token) const {return entries.contains(token);}

    std::unordered_map<Token, std::shared_ptr<StructuredValue>>::iterator begin() {return entries.begin();}
    std::unordered_map<Token, std::shared_ptr<StructuredValue>>::iterator end() {return entries.end();}
    [[nodiscard]] std::unordered_map<Token, std::shared_ptr<StructuredValue>>::const_iterator begin() const {return entries.begin();}
    [[nodiscard]] std::unordered_map<Token, std::shared_ptr<StructuredValue>>::const_iterator end() const {return entries.end();}
};


#endif // HAD_XLR8_STRUCTURED_DICTIONARY_H
#undef IN_XLR8_STRUCTURED_DICTIONARY_H
