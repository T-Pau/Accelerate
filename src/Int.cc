/*
Int.cc --

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

#include "Int.h"

size_t Int::minimum_byte_size(uint64_t value) {
    if (value > std::numeric_limits<uint32_t>::max()) {
        return 8;
    }
    else if (value > std::numeric_limits<uint16_t>::max()) {
        return 4;
    }
    else if (value > std::numeric_limits<uint8_t>::max()) {
        return 2;
    }
    else {
        return 1;
    }
}

size_t Int::minimum_byte_size(int64_t value) {
    if (value >= 0) {
        if (value > std::numeric_limits<int32_t>::max()) {
            return 8;
        }
        else if (value > std::numeric_limits<int16_t>::max()) {
            return 4;
        }
        else if (value > std::numeric_limits<int8_t>::max()) {
            return 2;
        }
        else {
            return 1;
        }
    }
    else {
        if (value < std::numeric_limits<int32_t>::min()) {
            return 8;
        }
        else if (value < std::numeric_limits<int16_t>::min()) {
            return 4;
        }
        else if (value < std::numeric_limits<int8_t>::min()) {
            return 2;
        }
        else {
            return 1;
        }
    }
}

void Int::encode(std::string &bytes, uint64_t value, uint64_t size, uint64_t byte_order) {
    uint64_t mask = 10000000;
    for (auto index = 0; index < 8; index += 1) {
        uint64_t byte_index = (byte_order / mask) % 10 - 1;
        mask /= 10;
        if (byte_index >= size) {
            continue;
        }
        uint64_t byte = (value >> (byte_index * 8)) & 0xff;
        bytes += static_cast<char>(byte);
    }
}

size_t Int::align(size_t value, size_t alignment) {
    if (alignment == 0) {
        return value;
    }

    size_t rest = value % alignment;
    if (rest == 0) {
        return value;
    }
    return value + (alignment - rest);
}
