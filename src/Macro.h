//
// Created by Dieter Baron on 29.05.23.
//

#ifndef MACRO_H
#define MACRO_H

#include "Body.h"
#include "Callable.h"
#include "Symbol.h"

class Macro: public Callable {
  public:
    Body expand(const std::vector<Expression>& arguments) const;
    void serialize(std::ostream& stream) const;

    Body body;
};

std::ostream& operator<<(std::ostream& stream, const Macro& macro);

#endif // MACRO_H
