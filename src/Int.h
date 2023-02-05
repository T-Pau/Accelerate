//
// Created by Dieter Baron on 29.01.23.
//

#ifndef INT_H
#define INT_H

#include <cstdint>
#include <vector>

class Int {
public:
    static void encode(std::vector<uint8_t>& bytes, int64_t value, uint64_t byte_order);
    static size_t minimum_byte_size(int64_t value);
};


#endif //INT_H
