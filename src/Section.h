//
// Created by Dieter Baron on 23.01.23.
//

#ifndef SECTION_H
#define SECTION_H

#include <cstdint>
#include <vector>

class Section {
public:
    enum AccessType {
        RESERVE_ONLY,
        READ_ONLY,
        READ_WRITE
    };

    class Block {
        uint64_t bank;
        uint64_t start;
        uint64_t size;
    };

    AccessType access;
    std::vector<Block> blocks;
};


#endif // SECTION_H
