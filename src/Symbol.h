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
    enum Visibility {
        NONE,
        LOCAL,
        GLOBAL
    };

    Symbol(symbol_t section, Visibility visibility, Token name): section(section), visibility(visibility), name(name) {}

    [[nodiscard]] bool is_reservation() const {return data.empty();}
    [[nodiscard]] bool empty() const {return is_reservation() && size == 0;}

    void append(const ExpressionList& list) {data.append(list);}
    void append(const std::shared_ptr<ExpressionNode>& expression) {data.append(expression);}

    symbol_t section;
    Visibility visibility;
    Token name;
    uint64_t alignment = 0;
    uint64_t size = 0;

    ExpressionList data;
};

#endif // SYMBOL_H
