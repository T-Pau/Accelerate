//
// Created by Dieter Baron on 24.01.23.
//

#include "MemoryMap.h"
#include "Exception.h"

void MemoryMap::Range::reduce(uint64_t amount) {
    if (amount > size) {
        throw Exception("not enough space");
    }

    size -= amount;
    offset -= amount;
}
