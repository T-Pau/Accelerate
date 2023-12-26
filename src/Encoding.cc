/*
Encoding.cc --

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

#include "Encoding.h"

#include "Exception.h"
#include "Target.h"

Encoding::Encoding(const Value& value): encoding{nullptr} {
    if (value.is_binary()) {
        encoding = BinaryEncoding();
    }
    else if (value.is_integer()) {
        encoding = IntegerEncoding(value);
    }
    else if (value.is_string()) {
        auto string_encoding = Target::current_target->default_string_encoding;
        if (!string_encoding) {
            throw Exception("no default string encoding specified");
        }
        encoding = string_encoding;
    }
    else {
        throw Exception("can't encode %s", value.type_name().c_str());
    }
}

void Encoding::encode(std::string& bytes, const Value& value) const {
    if (is_binary_encoding()) {
        as_binary_encoding().encode(bytes, value);
    }
    else if (is_integer_encoding()) {
        as_integer_encoding().encode(bytes, value);
    }
    else {
        as_string_encoding()->encode(bytes, value);
    }
}

size_t Encoding::encoded_size(const Value& value) const {
    if (is_binary_encoding()) {
        return as_binary_encoding().encoded_size(value);
    }
    else if (is_integer_encoding()) {
        return as_integer_encoding().byte_size();
    }
    else {
        return as_string_encoding()->encoded_size(value);
    }
}

bool Encoding::fits(const Value& value) const {
    if (is_integer_encoding()) {
        return as_integer_encoding().fits(value);
    }
    else {
        return true;
    }
}

bool Encoding::is_natural_encoding(const Value& value) const {
    if (is_integer_encoding()) {
        return as_integer_encoding().is_natural_encoding(value);
    }
    else if (is_string_encoding()) {
        return as_string_encoding() == Target::current_target->default_string_encoding;
    }
    else {
        return true;
    }
}

void Encoding::serialize(std::ostream& stream) const {
    if (is_binary_encoding()) {
    }
    else if (is_integer_encoding()) {
        stream << as_integer_encoding();
    }
    else {
        stream << (*as_string_encoding());
    }
}

bool Encoding::operator==(const Encoding& other) const {
    if (is_integer_encoding() != other.is_integer_encoding()) {
        return false;
    }
    return encoding == other.encoding;
}


std::ostream& operator<<(std::ostream& stream, const Encoding& encoding) {
    encoding.serialize(stream);
    return stream;
}
