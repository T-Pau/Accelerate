/*
Range.cc -- 

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

#include "Range.h"

#include <algorithm>

#include "Exception.h"

Range Range::intersect(const Range &other) const {
    auto new_start = std::max(start, other.start);
    auto new_end = std::min(end(), other.end());
    auto new_size = std::max(new_end - new_start, static_cast<uint64_t>(0));

    return {new_start, new_size};
}

void Range::add_left(uint64_t amount) {
    start -= amount;
    size += amount;
}

void Range::remove_left(uint64_t amount) {
    if (amount > size) {
        throw Exception("out of memory");
    }

    start += amount;
    size -= amount;
}

void Range::remove_right(uint64_t amount) {
    if (amount > size) {
        throw Exception("out of memory");
    }

    size -= amount;
}

void Range::set_start(uint64_t new_start) {
    if (new_start > end()) {
        throw Exception("out of memory");
    }
    size += start - new_start;
    start = new_start;
}

void Range::set_end(uint64_t new_end) {
    if (new_end < start) {
        throw Exception("out of memory");
    }
    size = new_end - start;
}

Range Range::add(const Range &other) const {
    if (size == 0) {
        return other;
    }
    if (other.size == 0) {
        return *this;
    }

    auto new_start = std::min(start, other.start);
    auto new_end = std::max(end(), other.end());
    auto new_size = new_end - new_start;

    return {new_start, new_size};
}

bool Range::operator<(const Range &other) const {
    if (start != other.start) {
        return start < other.start;
    }
    return size < other.size;
}
