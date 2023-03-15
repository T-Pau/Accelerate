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
#include "Int.h"

Memory::Bank::Bank(Range range): range(range) {
    memory = std::string(range.size, 0);
    blocks.emplace_back(FREE, range);
}

void Memory::Bank::copy(uint64_t address, const std::string& data) {
    memory.replace(address, data.size(), data);
}

std::optional<uint64_t> Memory::Bank::allocate(const Range &allowed_range, Memory::Allocation allocation, uint64_t alignment, uint64_t size) {
    if (allowed_range.size < size) {
        return {};
    }

    auto it = blocks.begin();

    while (it != blocks.end()) {
        if (it->allocation != FREE) {
            continue;
        }
        auto available_range = it->range.intersect(allowed_range);
        available_range.start = Int::align(available_range.start, alignment);

        if (available_range.size < size) {
            continue;
        }
        if (it->range.size == size) {
            it->allocation = allocation;
        }
        else if (it->range.start == available_range.start) {
            it->range.remove_left(size);
            it = blocks.insert(it, Block(allocation, {available_range.start, size}));
        }
        else {
            auto old_start = it->range.start;
            it->range.set_start(available_range.start + size);
            it = blocks.insert(it, Block(allocation, {available_range.start, size}));
            blocks.insert(it, Block(FREE, {old_start, available_range.start}));
        }

        if (it != blocks.begin()) {
            auto previous = std::prev(it);
            if (previous->allocation == it->allocation && previous->range.end() == it->range.start) {
                it->range.set_start(previous->range.start);
                blocks.erase(previous);
            }
        }
        if (it != blocks.end()) {
            auto next = std::next(it);
            if (next->allocation == it->allocation && next->range.start == it->range.end()) {
                it->range.set_end(next->range.end());
                blocks.erase(next);
            }
        }

        return available_range.start;
    }

    return {};
}

Range Memory::Bank::data_range() const {
    bool have_data = false;
    uint64_t start = 0;
    uint64_t end = 0;

    for (const auto& block: blocks) {
        if (block.allocation == DATA) {
            if (!have_data) {
                have_data = true;
                start = block.range.start;
            }
            end = block.range.end();
        }
    }

    if (have_data) {
        return {start, end - start};
    }
    else {
        return {};
    }
}


std::string Memory::Bank::data(const Range& range) const {
    return memory.substr(range.start, range.size);
}


Memory::Memory(const std::vector<Range>& bank_ranges) {
    for (const auto& bank_range: bank_ranges) {
        banks.emplace_back(bank_range);
    }
}

