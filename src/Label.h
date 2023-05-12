//
// Created by Dieter Baron on 11.05.23.
//

#ifndef LABEL_H
#define LABEL_H

#include "Symbol.h"
#include "SizeRange.h"

class Label {
public:
    Label(Symbol name): name(name) {}
    Label(Symbol name, SizeRange offset): name(name), offset(offset) {}

    const Symbol name;
    mutable SizeRange offset;
};


#endif // LABEL_H
