/*
SizeRange.h --

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

#ifndef SIZE_RANGE_H
#define SIZE_RANGE_H

#include <cstdint>
#include <optional>
#include <ostream>

#include "Value.h"

class SizeRange {
public:
    SizeRange() = default;
    explicit SizeRange(uint64_t size): minimum(size), maximum(size) {}
    SizeRange(uint64_t minimum, std::optional<uint64_t> maximum): minimum(minimum), maximum(maximum) {}

    [[nodiscard]] std::optional<Value> maximum_value() const {return maximum ? Value(*maximum) : std::optional<Value>();}
    [[nodiscard]] Value minimum_value() const {return Value(minimum);}
    [[nodiscard]] std::optional<Value> value() const;
    [[nodiscard]] std::optional<uint64_t> size() const;

    bool operator==(const SizeRange&other) const {return minimum == other.minimum && maximum == other.maximum;}
    bool operator!=(const SizeRange&other) const {return !(*this == other);}
    SizeRange operator-(const SizeRange& other) const;
    SizeRange operator+(const SizeRange& other) const;
    SizeRange operator+=(const SizeRange& other) {*this = *this + other; return *this;}

    uint64_t minimum = 0;
    std::optional<uint64_t> maximum = 0;
};

std::ostream& operator<<(std::ostream& stream, const SizeRange& size_range);

#endif // SIZE_RANGE_H
