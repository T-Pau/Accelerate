//
// Created by Dieter Baron on 29.05.23.
//

#ifndef FUNCTION_H
#define FUNCTION_H

#include "Callable.h"

class Function: public Callable {
  public:
    Function(Token name, const std::shared_ptr<ParsedValue>& definition);
    Function(Token name, Visibility visibility, Arguments arguments, Expression definition): Callable(name, visibility, std::move(arguments)), definition(definition) {}
    Expression call(const std::vector<Expression>& arguments);
    void serialize(std::ostream& stream) const;

    Expression definition;

  private:
    static void initialize();

    static bool initialized;
    static Token token_definition;
};

std::ostream& operator<<(std::ostream& stream, const Function& function);

#endif // FUNCTION_H
