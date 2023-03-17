//
// Created by Dieter Baron on 17.03.23.
//

#ifndef OUTPUT_ELEMENT_H
#define OUTPUT_ELEMENT_H

#include "ExpressionList.h"

class OutputElement {
public:
    enum Type {
        DATA,
        MEMORY
    };

    OutputElement(Type type, ExpressionList arguments): type(type), arguments(std::move(arguments)) {}

    Type type;
    ExpressionList arguments;
};


#endif // OUTPUT_ELEMENT_H
