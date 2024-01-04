/*
BinaryEncoder.h --

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

#ifndef BINARY_ENCODER_H
#define BINARY_ENCODER_H

#include "BaseEncoder.h"
#include "Value.h"

class BinaryEncoder: public BaseEncoder {
public:
    void encode(std::string& bytes, const Value& value) const override;
    [[nodiscard]] size_t encoded_size(const Value& value) const override;
    [[nodiscard]] bool fits(const Value& value) const override {return value.is_binary();}
    [[nodiscard]] bool is_natural_encoder(const Value& value) const override {return value.is_binary();}
    void serialize(std::ostream& stream) const override {}
    [[nodiscard]] SizeRange size_range() const override {return SizeRange{0, {}};}
    bool operator==(const Encoder&) const override;

    bool operator==(const BinaryEncoder&) const {return true;}
};

#endif // BINARY_ENCODER_H
