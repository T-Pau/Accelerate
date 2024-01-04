#ifndef ENCODER_H
#define ENCODER_H

/*
Encoder.h --

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

#include <variant>

#include "BaseEncoder.h"
#include "BinaryEncoder.h"
#include "IntegerEncoder.h"
#include "StringEncoder.h"
\
class Encoder {
  public:
    explicit Encoder(const IntegerEncoder& encoder): encoder{std::make_shared<IntegerEncoder>(encoder)} {}
    explicit Encoder(const StringEncoding* string_encoding, std::optional<size_t> size = {}): encoder{std::make_shared<StringEncoder>(string_encoding, size)} {}
    explicit Encoder(const Value& value);
    explicit Encoder(Tokenizer& tokenizer);

    [[nodiscard]] const BinaryEncoder* as_binary_encoder() const {return std::dynamic_pointer_cast<BinaryEncoder>(encoder).get();}
    [[nodiscard]] const IntegerEncoder* as_integer_encoder() const {return std::dynamic_pointer_cast<IntegerEncoder>(encoder).get();}
    [[nodiscard]] const StringEncoder* as_string_encoder() const {return std::dynamic_pointer_cast<StringEncoder>(encoder).get();}
    [[nodiscard]] bool is_binary_encoder() const {return as_binary_encoder();}
    [[nodiscard]] bool is_integer_encoder() const {return as_integer_encoder();}
    [[nodiscard]] bool is_string_encoder() const {return as_string_encoder();}
    [[nodiscard]] bool operator==(const Encoder& other) const {return *encoder == other;}
    [[nodiscard]] bool operator==(const IntegerEncoder& other) const {return is_integer_encoder() && *as_integer_encoder() == other;}

    void encode(std::string& bytes, const Value& value) const {return encoder->encode(bytes, value);}
    [[nodiscard]] size_t encoded_size(const Value& value) const {return encoder->encoded_size(value);}
    [[nodiscard]] bool fits(const Value& value) const {return encoder->fits(value);}
    [[nodiscard]] bool is_natural_encoder(const Value& value) const {return encoder->is_natural_encoder(value);}
    void serialize(std::ostream& stream) const {return encoder->serialize(stream);}
    [[nodiscard]] SizeRange size_range() const {return encoder->size_range();}

  private:
    std::shared_ptr<BaseEncoder> encoder;
};

std::ostream& operator<<(std::ostream& stream, const Encoder& encoder);

#endif // ENCODER_H
