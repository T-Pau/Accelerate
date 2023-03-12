//
// Created by Dieter Baron on 29.01.23.
//

#include "Int.h"

size_t Int::minimum_byte_size(int64_t value) {
    if (value > std::numeric_limits<uint32_t>::max()) {
        return 8;
    }
    else if (value > std::numeric_limits<uint16_t>::max()) {
        return 4;
    }
    else if (value > std::numeric_limits<uint8_t>::max()) {
        return 2;
    }
    else {
        return 1;
    }
}

void Int::encode(std::vector<uint8_t> &bytes, int64_t value, uint64_t byte_order) {

}

size_t Int::align(size_t value, size_t alignment) {
    if (alignment == 0) {
        return value;
    }

    size_t rest = value % alignment;
    if (rest == 0) {
        return value;
    }
    return value + (alignment - rest);
}
