/*
Memory.cc -- 

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

#include "Memory.h"
#include "Exception.h"
#include "Int.h"

Memory::Bank::Bank(Memory::Range range): range(range) {
    memory = std::string(range.size(), 0);
    blocks.emplace_back(FREE, range);
}

size_t Memory::Bank::allocate(const Memory::Range &allowed_range, size_t alignment, const std::string &data) {
    size_t address = allocate(allowed_range, DATA, alignment, data.size());

    memory.replace(range.start, data.size(), data);

    return address;
}

size_t Memory::Bank::allocate(const Memory::Range &allowed_range, Memory::Allocation allocation, size_t alignment, size_t size) {
    if (allowed_range.size() < size) {
        throw Exception("out of memory");
    }

    auto it = blocks.begin();

    while (it != blocks.end()) {
        if (it->allocation != FREE) {
            continue;
        }
        auto available_range = it->range.intersect(allowed_range);
        available_range.start = Int::align(available_range.start, alignment);

        if (available_range.size() < size) {
            continue;
        }
        if (it->range.size() == size) {
            it->allocation = allocation;
        }
        else if (it->range.start == available_range.start) {
            it->range.start += size;
            it = blocks.insert(it, Block(allocation, {available_range.start, size}));
        }
        else {
            auto old_start = it->range.start;
            it->range.start = available_range.start + size;
            it = blocks.insert(it, Block(allocation, {available_range.start, size}));
            blocks.insert(it, Block(FREE, {old_start, available_range.start}));
        }

        if (it != blocks.begin()) {
            auto previous = std::prev(it);
            if (previous->allocation == it->allocation && previous->range.end == it->range.start) {
                it->range.start = previous->range.start;
                blocks.erase(previous);
            }
        }
        if (it != blocks.end()) {
            auto next = std::next(it);
            if (next->allocation == it->allocation && next->range.start == it->range.end) {
                it->range.end = next->range.end;
                blocks.erase(next);
            }
        }

        return available_range.start;
    }

    throw Exception("out of memory");
}

size_t Memory::Bank::data_start() const {
    for (const auto& block: blocks) {
        if (block.allocation == DATA) {
            return block.range.start;
        }
    }

    return std::numeric_limits<size_t>::max();
}

size_t Memory::Bank::data_end() const {
    size_t end = 0;

    for (const auto& block: blocks) {
        if (block.allocation == DATA) {
            end = block.range.end;
        }
    }

    return end;
}

Memory::Range Memory::Range::intersect(const Memory::Range &other) const {
    auto range = Range(std::max(start, other.start), std::min(end, other.end));
    if (range.start > range.end) {
        range.end = 0;
    }

    return range;
}
