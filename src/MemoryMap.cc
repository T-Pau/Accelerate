/*
MemoryMap.cc --

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

#include "MemoryMap.h"
#include "Exception.h"

const std::vector<MemoryMap::Block> *MemoryMap::segment(symbol_t name) const {
    auto it = segments.find(name);

    if (it == segments.end()) {
        return nullptr;
    }
    else {
        return &it->second;
    }
}


const MemoryMap::Section *MemoryMap::section(symbol_t name) const {
    auto it = sections.find(name);

    if (it == sections.end()) {
        return nullptr;
    }
    else {
        return &it->second;
    }
}


Memory MemoryMap::initialize_memory() const {
    std::vector<Range> bank_ranges;

    for (auto const& section: sections) {
        for (auto const& block: section.second.blocks) {
            if (bank_ranges.size() < block.bank + 1) {
                bank_ranges.resize(block.bank + 1);
            }
            bank_ranges[block.bank] = bank_ranges[block.bank].add(block.range);
        }
    }

    return Memory(bank_ranges);
}

bool MemoryMap::access_type_less(MemoryMap::AccessType a, MemoryMap::AccessType b) {
    switch (a) {
        case RESERVE_ONLY:
            return false;

        case READ_ONLY:
            switch (b) {
                case RESERVE_ONLY:
                case READ_ONLY:
                    return false;
                case READ_WRITE:
                    return true;
            }
            break;

        case READ_WRITE:
            switch (b) {
                case RESERVE_ONLY:
                case READ_ONLY:
                    return true;
                case READ_WRITE:
                    return false;
            }
            return true;
    }
}


bool MemoryMap::Block::operator<(const MemoryMap::Block &other) const {
    if (bank != other.bank) {
        return bank < other.bank;
    }
    return range < other.range;
}


bool MemoryMap::Block::operator==(const MemoryMap::Block &other) const {
    return bank == other.bank && range == other.range;
}


MemoryMap::Section::Section(symbol_t name, MemoryMap::AccessType access, std::vector<Block> raw_blocks): name(name), access(access) {
    std::sort(raw_blocks.begin(), raw_blocks.end());
    Block* previous = nullptr;

    for (const auto& block: raw_blocks) {
        if (previous != nullptr && block.bank == previous->bank && block.range.start <= previous->range.end()) {
            size += block.range.end() - previous->range.end();
            previous->range.set_end(block.range.end());
        }
        else {
            size += block.range.size;
            previous = &blocks.emplace_back(block);
        }
        address_size = std::max(address_size, Int::minimum_byte_size(static_cast<int64_t>(previous->range.end() - 1)));
    }
}


bool MemoryMap::Section::operator<(const MemoryMap::Section &other) const {
    if (size != other.size) {
        return size < other.size;
    }

    if (access != other.access) {
        return access_type_less(access, other.access);
    }

    auto it = blocks.begin();
    auto it_other = other.blocks.begin();
    while (it != blocks.end() && it_other != other.blocks.end()) {
        if (*it != *it_other) {
            return *it < *it_other;
        }
        it++;
        it_other++;
    }

    if (it == blocks.end()) {
        if (it_other != other.blocks.end()) {
            return true;
        }
        return SymbolTable::global_less(name, other.name);
    }

    return false;
}
