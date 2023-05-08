//
// Created by Dieter Baron on 07.05.23.
//

#ifndef FUNCTION_EXPRESSION_H
#define FUNCTION_EXPRESSION_H

#include "Expression.h"

class FunctionExpression: public Expression {
public:
    FunctionExpression(Symbol name, std::vector<std::shared_ptr<Expression>> arguments): name(name), arguments(std::move(arguments)) {}
    static std::shared_ptr<Expression> create(Symbol name, const std::vector<std::shared_ptr<Expression>>& arguments);

    [[nodiscard]] Type type() const override {return FUNCTION;}

    void collect_variables(std::vector<Symbol>& variables) const override;

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override;

    void serialize_sub(std::ostream& stream) const override;

private:
    Symbol name;
    std::vector<std::shared_ptr<Expression>> arguments;
};


#endif // FUNCTION_EXPRESSION_H
