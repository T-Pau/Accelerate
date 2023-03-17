//
// Created by Dieter Baron on 24.01.23.
//

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Memory.h"
#include "ExpressionList.h"
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
        Block(uint64_t bank, uint64_t start, uint64_t size): bank(bank), range(start, size) {}

        uint64_t bank;
        Range range;

        bool operator<(const Block& other) const;
        bool operator==(const Block& other) const;
    };

    class Section {
    public:
        Section() = default;
        Section(symbol_t name, AccessType access, std::vector<Block> raw_blocks);

        bool operator<(const Section& other) const;

        symbol_t name = 0;
        AccessType access = READ_WRITE;
        std::vector<Block> blocks;
        size_t size = 0;
    };


    [[nodiscard]] const std::vector<Block>* segment(symbol_t name) const;
    [[nodiscard]] const Section* section(symbol_t name) const;
    [[nodiscard]] Memory initialize_memory() const;

    void add_section(Section section) {sections[section.name] = std::move(section);}
    void add_segment(symbol_t name, std::vector<Block> segment) {segments[name] = std::move(segment);}

    static bool access_type_less(AccessType a, AccessType b);

    std::unordered_map<symbol_t, Section> sections;
    std::unordered_map<symbol_t, std::vector<Block>> segments;
};


#endif // MEMORY_MAP_H
