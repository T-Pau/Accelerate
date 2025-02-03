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

#include <iomanip>
#include <ostream>
#include <sstream>

#include "Exception.h"
#include <iostream>

Memory::Bank::Bank(Range range, uint8_t fill_byte) : range(range) {
    memory = std::string(range.size, static_cast<char>(fill_byte));
    blocks.emplace_back(FREE, range);
}

void Memory::Bank::copy(uint64_t address, const std::string& data) {
    auto target_range = Range(address, data.size());
    if (!range.contains(target_range)) {
        auto str = std::stringstream{};
        str << "data " << target_range << " out of range " << range;
        throw Exception(str.str());
    }
    memory.replace(offset(address), data.size(), data);
}

std::optional<uint64_t> Memory::Bank::allocate(const Range& allowed_range, Memory::Allocation allocation, uint64_t alignment, uint64_t size) {
    if (allowed_range.size < size) {
        return {};
    }

    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (it->allocation != FREE) {
            continue;
        }
        auto available_range = it->range.intersect(allowed_range);
        available_range.align(alignment);

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
            it = blocks.insert(it, Block(FREE, {old_start, available_range.start - old_start}));
            it = std::next(it);
        }

        if (it != blocks.begin()) {
            auto previous = std::prev(it);
            if (previous->allocation == it->allocation && previous->range.end() + 1 == it->range.start) {
                previous->range.set_end(it->range.end());
                blocks.erase(it);
                it = previous;
            }
        }
        if (it != blocks.end()) {
            auto next = std::next(it);
            if (next->allocation == it->allocation && next->range.start == it->range.end() + 1) {
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

    for (const auto& block : blocks) {
        if (block.allocation == DATA) {
            if (!have_data) {
                have_data = true;
                start = block.range.start;
            }
            end = block.range.end();
        }
    }

    if (have_data) {
        return {start, end - start + 1};
    }
    else {
        return {};
    }
}

std::string Memory::Bank::data(const Range& requested_range) const {
    auto result = std::string(requested_range.size, 0);
    auto overlap_range = requested_range.intersect(range);
    if (!overlap_range.empty()) {
        result.replace(overlap_range.start - requested_range.start, overlap_range.size, memory, offset(overlap_range.start), overlap_range.size);
    }
    return result;
}

void Memory::Bank::debug_blocks(std::ostream& stream) const {
    stream << "allocation blocks:" << std::endl;
    for (const auto& block : blocks) {
        stream << std::hex << std::setfill('0') << "  " << std::setw(4) << block.range.start << "-" << std::setw(4) << block.range.end() << ": ";
        switch (block.allocation) {
            case DATA:
                stream << "data";
                break;
            case FREE:
                stream << "free";
                break;
            case RESERVED:
                stream << "reserved";
                break;
        }
        stream << std::endl;
    }
}

Memory::Memory(const std::vector<Range>& bank_ranges, uint8_t fill_byte) {
    for (const auto& bank_range : bank_ranges) {
        banks.emplace_back(bank_range, fill_byte);
    }
}
