/*
IntegerEncoder.cc --

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

#include "IntegerEncoder.h"

#include "Encoder.h"
#include "Exception.h"
#include "Int.h"
#include "Target.h"

const uint64_t IntegerEncoder::big_endian_byte_order = 87654321;
const uint64_t IntegerEncoder::little_endian_byte_order = 12345678;

uint64_t IntegerEncoder::default_byte_order() {
    return Target::current_target && Target::current_target->cpu ? Target::current_target->cpu->byte_order : little_endian_byte_order;
}

std::ostream& operator<<(std::ostream& stream, const IntegerEncoder& encoding) {
    encoding.serialize(stream);
    return stream;
}

bool IntegerEncoder::operator==(const IntegerEncoder& other) const { return type == other.type && size == other.size && explicit_byte_order == other.explicit_byte_order; }

void IntegerEncoder::encode(std::string& bytes, const Value& value) const {
    if (!fits(value)) {
        throw Exception("value overflow");
    }

    switch (type) {
        case SIGNED: {
            int64_t signed_value = value.signed_value();
            Int::encode(bytes, *(reinterpret_cast<uint64_t*>(&signed_value)), size ? *size : encoded_size(value), byte_order());
            break;
        }

        case UNSIGNED:
            Int::encode(bytes, value.unsigned_value(), size ? *size : encoded_size(value), byte_order());
            break;
    }
}

size_t IntegerEncoder::encoded_size(const Value& value) const {
    if (size) {
        return *size;
    }
    else {
        if (const auto default_size = value.default_size()) {
            return *default_size;
        }
        else {
            throw Exception("can't encode %s", value.type_name().c_str());
        }
    }
}

bool IntegerEncoder::fits(const Value& value) const { return value.is_integer() && value >= minimum_value() && value <= maximum_value(); }

bool IntegerEncoder::is_natural_encoder(const Value& value) const {
    switch (type) {
        case SIGNED:
            return value.is_signed() && value.default_size() == size && byte_order() == default_byte_order();

        case UNSIGNED:
            return value.is_unsigned() && value.default_size() == size && byte_order() == default_byte_order();
    }
    throw Exception("internal error: invalid integer encoder type %d", type);
}

void IntegerEncoder::serialize(std::ostream& stream) const {
    if (byte_order() == default_byte_order() && size) {
        switch (type) {
            case SIGNED:
                stream << ":-" << *size;
                break;

            case UNSIGNED:
                stream << ":" << *size;
                break;
        }
    }
    else {
        if (type == SIGNED && !size) {
            stream << ":-";
        }
        else {
            stream << ":";
        }
        // TODO: handle signed
        if (byte_order() == little_endian_byte_order) {
            stream << "little_endian";
        }
        else if (byte_order() == big_endian_byte_order) {
            stream << "big_endian";
        }
        else {
            // TODO: encode strange non-default byte_orders
        }
        if (size) {
            stream << "(";
            if (type == SIGNED) {
                stream << "-";
            }
            stream << *size << ")";
        }
    }
}

SizeRange IntegerEncoder::size_range() const {
    if (size) {
        return SizeRange{*size};
    }
    return {0, 8};
}

IntegerEncoder::IntegerEncoder(const Value& value) {
    switch (value.type()) {
        case Value::BINARY:
        case Value::BOOLEAN:
        case Value::FLOAT:
        case Value::STRING:
        case Value::VOID:
            throw Exception("can't encode %s", value.type_name().c_str());

        case Value::SIGNED:
            type = SIGNED;
            break;

        case Value::UNSIGNED:
            type = UNSIGNED;
            break;


        case Value::NUMBER:
        case Value::INTEGER:
            throw Exception("internal error: value can't have abstract type %s", value.type_name().c_str());
    }
    size = *value.default_size();
}

std::optional<Value> IntegerEncoder::minimum_value() const {
    switch (type) {
        case SIGNED:
            if (!size || size == 8) {
                return Value(std::numeric_limits<int64_t>::min());
            }
            return Value(-(static_cast<int64_t>(1) << (*size * 8 - 1)));

        case UNSIGNED:
            return Value(uint64_t{0});
    }

    throw Exception("internal error: invalid integer encoder type %d", type);
}

bool IntegerEncoder::operator==(const Encoder& other) const {
    if (other.is_integer_encoder()) {
        return *this == *other.as_integer_encoder();
    }
    else {
        return false;
    }
}

std::optional<Value> IntegerEncoder::maximum_value() const {
    switch (type) {
        case SIGNED:
            return Value((int64_t{1} << (size.value_or(8) * 8 - 1)) - 1);

        case UNSIGNED:
            if (!size || size == 8) {
                return Value(std::numeric_limits<uint64_t>::max());
            }
            return Value((uint64_t{1} << (*size * 8)) - 1);
    }

    throw Exception("internal error: invalid integer encoder type %d", type);
}
