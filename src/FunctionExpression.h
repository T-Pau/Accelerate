//
// Created by Dieter Baron on 07.05.23.
//

#ifndef FUNCTION_EXPRESSION_H
#define FUNCTION_EXPRESSION_H

#include "Expression.h"

class FunctionExpression: public BaseExpression {
public:
    FunctionExpression(Symbol name, std::vector<Expression> arguments): name(name), arguments(std::move(arguments)) {}

    void collect_objects(std::unordered_set<Object*>& objects) const override;

protected:
    static Expression create(Symbol name, const std::vector<Expression>& arguments);
    [[nodiscard]] std::optional<Expression> evaluated(const Environment &environment) const override;

    void serialize_sub(std::ostream& stream) const override;

    friend class Expression;

private:
    Symbol name;
    std::vector<Expression> arguments;
};


#endif // FUNCTION_EXPRESSION_H
