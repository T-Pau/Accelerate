//
// Created by Dieter Baron on 24.01.23.
//

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "SymbolTable.h"

class MemoryMap {
public:
    enum AccessType {
        RESERVE_ONLY,
        READ_ONLY,
        READ_WRITE
    };

    class Block {
    public:
        Block(uint64_t bank, uint64_t start, uint64_t size): bank(bank), start(start), size(size) {}

        uint64_t bank;
        uint64_t start;
        uint64_t size;
    };

    class Section {
    public:
        Section() = default;
        Section(AccessType access, std::vector<Block> blocks): access(access), blocks(std::move(blocks)) {}

        AccessType access = READ_WRITE;
        std::vector<Block> blocks;
    };

    [[nodiscard]] const std::vector<Block>* segment(symbol_t name) const;
    [[nodiscard]] const Section* section(symbol_t name) const;

    void add_section(symbol_t name, Section section) {sections[name] = std::move(section);}
    void add_segment(symbol_t name, std::vector<Block> segment) {segments[name] = std::move(segment);}

    std::unordered_map<symbol_t, Section> sections;
    std::unordered_map<symbol_t, std::vector<Block>> segments;
};


#endif // MEMORY_MAP_H
