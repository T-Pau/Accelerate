//
// Created by Dieter Baron on 24.01.23.
//

#ifndef SYMBOL_H
#define SYMBOL_H

#include <vector>

#include "MemoryMap.h"
#include "ExpressionList.h"

class Symbol {
public:
    [[nodiscard]] bool is_reservation() const {return data.empty();}

    Token name;
    uint64_t size;

    ExpressionList data;
};

#endif // SYMBOL_H
