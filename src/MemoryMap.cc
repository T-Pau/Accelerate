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
            auto& bank_range = bank_ranges[block.bank];
        }
    }

    return Memory(bank_ranges);
}



