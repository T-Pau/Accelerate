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



