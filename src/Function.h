//
// Created by Dieter Baron on 29.05.23.
//

#ifndef FUNCTION_H
#define FUNCTION_H

#include "Callable.h"

class Function: public Callable {
  public:
    Expression call(const std::vector<Expression>& arguments);
    void serialize(std::ostream& stream) const;

    Expression value;
};

std::ostream& operator<<(std::ostream& stream, const Function& function);

#endif // FUNCTION_H
