//
// Created by Dieter Baron on 29.01.23.
//

#ifndef INT_H
#define INT_H

#include <cstdint>
#include <string>

class Int {
public:
    static void encode(std::string& bytes, int64_t value, uint64_t size, uint64_t byte_order);
    static size_t minimum_byte_size(int64_t value);

    static size_t align(size_t value, size_t alignment);
};


#endif //INT_H
