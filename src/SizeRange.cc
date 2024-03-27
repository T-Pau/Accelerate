/*
SizeRange.cc --

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

#include "SizeRange.h"

SizeRange SizeRange::operator+(const SizeRange &other) const {
    if (!maximum || !other.maximum) {
        return {minimum + other.minimum, {}};
    }
    else {
        return {minimum + other.minimum, *maximum + *other.maximum};
    }
}


SizeRange SizeRange::operator-(const SizeRange &other) const {
    if (!other.maximum) {
        return {0, {}};
    }
    auto new_minimum = minimum - *other.maximum;
    if (!maximum) {
        return {new_minimum, {}};
    }
    else {
        return {new_minimum, *maximum - other.minimum};
    }
}


std::optional<uint64_t> SizeRange::size() const {
    if (has_size()) {
        return minimum;
    }
    else {
        return {};
    }
}

std::optional<Value> SizeRange::value() const {
    auto s = size();
    if (s) {
        return Value(*s);
    }
    else {
        return {};
    }
}

std::ostream& operator<<(std::ostream& stream, const SizeRange& size_range) {
    stream << "(" << size_range.minimum << ",";
    if (size_range.maximum.has_value()) {
        stream << " " << *size_range.maximum;
    }
    stream << ")";

    return stream;
}
