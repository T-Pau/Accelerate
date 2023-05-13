/*
Encoding.cc -- 

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

#include "Encoding.h"

#include "Exception.h"
#include "Int.h"

uint64_t Encoding::default_byte_order;

std::ostream& operator<<(std::ostream& stream, const Encoding& encoding) {
    encoding.serialize(stream);
    return stream;
}

bool Encoding::operator==(const Encoding &other) const {
    return type == other.type && size == other.size && explicit_byte_order == other.explicit_byte_order;
}

void Encoding::encode(std::string &bytes, const Value &value) const {
    if (!fits(value)) {
        throw Exception("value overflow");
    }

    switch (type) {
        case SIGNED: {
            int64_t signed_value = value.signed_value();
            Int::encode(bytes, *(reinterpret_cast<uint64_t*>(&signed_value)), size, byte_order());
            break;

        }

        case UNSIGNED:
            Int::encode(bytes, value.unsigned_value(), size, byte_order());
            break;
    }
}

bool Encoding::fits(const Value &value) const {
    switch (type) {
        case SIGNED:
        case UNSIGNED:
            return value.is_integer() && value >= minimum_value() && value <= maximum_value();
    }
}

bool Encoding::is_natural_encoding(const Value &value) const {
    switch (type) {
        case SIGNED:
            return value.is_signed() && value.default_size() == size && byte_order() == default_byte_order;

        case UNSIGNED:
            return value.is_unsigned() && value.default_size() == size && byte_order() == default_byte_order;
    }
}

void Encoding::serialize(std::ostream &stream) const {
    switch (type) {
        case SIGNED:
            stream << ":-" << size;
            break;

        case UNSIGNED:
            stream << ":" << size;
            break;
    }
    // TODO: encode byte_order if byte_order != default_byte_order
}

Encoding::Encoding(const Value value) {
    switch (value.type()) {
        case Value::BOOLEAN:
        case Value::FLOAT:
        case Value::VOID:
            throw Exception("can't encode %s", value.type_name().c_str());

        case Value::SIGNED:
            type = SIGNED;
            break;

        case Value::UNSIGNED:
            type = UNSIGNED;
            break;
    }
    size = value.default_size();
}

std::optional<Value> Encoding::minimum_value() const {
    switch (type) {
        case SIGNED:
            if (size == 8) {
                return Value(std::numeric_limits<int64_t>::min());
            }
            return Value(-(static_cast<int64_t>(1) << (size * 8 - 1)));

        case UNSIGNED:
            return Value(uint64_t(0));
    }
}

std::optional<Value> Encoding::maximum_value() const {
    switch (type) {
        case SIGNED:
            return Value((int64_t(1) << (size * 8 - 1)) - 1);

        case UNSIGNED:
            if (size == 8) {
                return Value(std::numeric_limits<uint64_t>::max());
            }
            return Value((uint64_t(1) << (size * 8)) - 1);
    }
}
