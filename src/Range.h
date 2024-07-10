/*
Range.h -- 

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

#ifndef ACCELERATE_RANGE_H
#define ACCELERATE_RANGE_H

#include "Int.h"
#include <cstdint>
#include <ostream>

class Range {
public:
    Range() = default;
    Range(uint64_t start, uint64_t size): start(start), size(size) {}

    [[nodiscard]] bool empty() const {return size == 0;}
    [[nodiscard]] uint64_t end() const {return empty() ? start : start - 1 + size;} // last byte included in range
    [[nodiscard]] bool contains(const Range& other) const {return other.start >= start && other.end() <= end();}
    [[nodiscard]] Range intersect(const Range& other) const;
    [[nodiscard]] Range add(const Range& other) const;

    bool operator==(const Range& other) const {return start == other.start && size == other.size;}
    bool operator<(const Range& other) const;
    bool operator!=(const Range& other) const {return !(*this == other);}

    void add_left(uint64_t amount);
    void add_right(uint64_t amount) {size += amount;}
    void align(uint alignment) {set_start(Int::align(start, alignment));}
    void remove_left(uint64_t amount);
    void remove_right(uint64_t amount);
    void set_start(uint64_t new_start);
    void set_end(uint64_t new_end);

    void serialize(std::ostream& stream) const;

    uint64_t start = 0;
    uint64_t size = 0;
};

std::ostream& operator<<(std::ostream& stream, const Range& range);

#endif //ACCELERATE_RANGE_H
