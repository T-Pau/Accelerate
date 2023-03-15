//
// Created by Dieter Baron on 24.01.23.
//

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
    // TODO: remove overlaps
    blocks = std::move(raw_blocks);
    size = 0;
    for (const auto& block: blocks) {
        size += block.range.size;
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
