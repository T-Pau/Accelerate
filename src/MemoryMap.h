/*
MemoryMap.h --

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

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Memory.h"
#include "ExpressionList.h"
#include "Symbol.h"

class MemoryMap {
public:
    enum AccessType {
        RESERVE_ONLY,
        READ_ONLY,
        READ_WRITE
    };

    class Block {
    public:
        Block(uint64_t bank, uint64_t start, uint64_t size): bank(bank), range(start, size) {}

        uint64_t bank;
        Range range;

        bool operator<(const Block& other) const;
        bool operator==(const Block& other) const;
    };

    class Section {
    public:
        Section() = default;
        Section(size_t address_size): address_size(address_size) {}
        Section(Symbol name, AccessType access, std::vector<Block> raw_blocks);

        bool operator<(const Section& other) const;

        [[nodiscard]] bool empty() const {return blocks.empty();}

        Symbol name;
        AccessType access = READ_WRITE;
        std::vector<Block> blocks;
        size_t size = 0;
        size_t address_size = 0;
    };


    [[nodiscard]] bool is_abstract() const;
    [[nodiscard]] bool is_compatible_with(const MemoryMap& other) const; // this has everything from other
    [[nodiscard]] bool has_section(Symbol name) const {return section(name) != nullptr;}
    [[nodiscard]] const std::vector<Block>* segment(Symbol name) const;
    [[nodiscard]] const Section* section(Symbol name) const;
    [[nodiscard]] Memory initialize_memory() const;

    void add_section(Section section) {sections[section.name] = std::move(section);}
    void add_segment(Symbol name, std::vector<Block> segment) {segments[name] = std::move(segment);}

    static bool access_type_less(AccessType a, AccessType b);

    std::unordered_map<Symbol, Section> sections;
    std::unordered_map<Symbol, std::vector<Block>> segments;
};


#endif // MEMORY_MAP_H
