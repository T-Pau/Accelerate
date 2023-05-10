//
// Created by Dieter Baron on 10.05.23.
//

#ifndef VOID_EXPRESSION_H
#define VOID_EXPRESSION_H

#include "BaseExpression.h"

class VoidExpression: public BaseExpression {
    void collect_variables(std::vector<Symbol> &variables) const override {}
    [[nodiscard]] std::optional<Expression> evaluated(const Environment &environment) const override;
    void serialize_sub(std::ostream &stream) const override {}
    [[nodiscard]] Type type() const override {return BaseExpression::VOID;}
};


#endif // VOID_EXPRESSION_H
