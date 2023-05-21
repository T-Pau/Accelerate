//
// Created by Dieter Baron on 11.05.23.
//

#ifndef LABEL_H
#define LABEL_H

#include "Symbol.h"
#include "SizeRange.h"

class Label {
public:
    explicit Label(Symbol name): name(name) {}
    Label(Symbol name, SizeRange offset): name(name), offset(offset) {}
    explicit Label(SizeRange offset): offset(offset) {}

    [[nodiscard]] bool is_named() const {return !name.empty();}

    const Symbol name;
    mutable SizeRange offset = SizeRange(0, {});
};

#endif // LABEL_H
