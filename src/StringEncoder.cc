/*
StringEncoder.cc -- 

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

#include "StringEncoder.h"

#include "StringEncoding.h"
#include "Target.h"

void StringEncoder::encode(std::string& bytes, const Value& value) const {
    string_encoding->encode(bytes, value, size);
}

size_t StringEncoder::encoded_size(const Value& value) const {
    if (size) {
        return *size;
    }
    else {
        return string_encoding->encoded_size(value);
    }
}

bool StringEncoder::fits(const Value& value) const {
    if (size) {
        return string_encoding->encoded_size(value) <= *size;
    }
    else {
        return true;
    }
}

bool StringEncoder::is_natural_encoder(const Value& value) const {
    return !size && string_encoding == Target::current_target->default_string_encoding;
}

void StringEncoder::serialize(std::ostream& stream) const {
    if (string_encoding != Target::current_target->default_string_encoding) {
        stream << ":" << *string_encoding;
    }
    if (size) {
        if (string_encoding == Target::current_target->default_string_encoding) {
            stream << ":string";
        }
        stream << "(" << *size << ")";
    }
}

SizeRange StringEncoder::size_range() const {
    if (size) {
        return SizeRange{*size};
    }
    else {
        return SizeRange{0, {}};
    }
}

bool StringEncoder::operator==(const Encoder& other) const {
    if (other.is_string_encoder()) {
        return *this == *other.as_string_encoder();
    }
    else {
        return false;
    }
}
