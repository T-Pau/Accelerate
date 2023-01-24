//
// Created by Dieter Baron on 24.01.23.
//

#ifndef RANGES_H
#define RANGES_H


#include <cstdint>

class MemoryMap {
public:
    class Range {
    public:
        uint64_t offset;
        uint64_t size;

        [[nodiscard]] bool empty() const {return size == 0;}
        void reduce(uint64_t amount);
    };
};


#endif // RANGES_H
