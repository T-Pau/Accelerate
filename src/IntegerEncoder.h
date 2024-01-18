/*
IntegerEncoder.h --

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

#ifndef INTEGER_ENCODER_H
#define INTEGER_ENCODER_H

#include "BaseEncoder.h"
#include "BaseExpression.h"
#include "Environment.h"
#include "Value.h"

class IntegerEncoder: public BaseEncoder {
public:
    enum Type {
        SIGNED,
        UNSIGNED
    };

    IntegerEncoder(Type type, std::optional<size_t> size, std::optional<uint64_t> byte_order = {}): type(type), size(size), explicit_byte_order(byte_order) {}
    explicit IntegerEncoder(const Value& value);

    [[nodiscard]] std::optional<size_t> byte_size() const {return size;}
    void encode(std::string& bytes, const Value& value) const override;
    [[nodiscard]] size_t encoded_size(const Value& value) const override;
    [[nodiscard]] bool fits(const Value& value) const override;
    [[nodiscard]] bool is_natural_encoder(const Value& value) const override;
    void serialize(std::ostream& stream) const override;
    [[nodiscard]] SizeRange size_range() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const;
    [[nodiscard]] std::optional<Value> minimum_value() const;
    bool operator==(const Encoder& other) const override;

    bool operator==(const IntegerEncoder& other) const;
    bool operator!=(const IntegerEncoder& other) const {return !(*this == other);}

    static uint64_t default_byte_order;
    static const uint64_t little_endian_byte_order;
    static const uint64_t big_endian_byte_order;

private:
    Type type;
    std::optional<size_t> size;
    std::optional<uint64_t> explicit_byte_order;

    [[nodiscard]] uint64_t byte_order() const {return explicit_byte_order.value_or(default_byte_order);}
};

std::ostream& operator<<(std::ostream& stream, const IntegerEncoder& encoding);

#endif // INTEGER_ENCODER_H
