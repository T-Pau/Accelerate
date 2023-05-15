//
// Created by Dieter Baron on 10.05.23.
//

#ifndef VOID_EXPRESSION_H
#define VOID_EXPRESSION_H

#include "BaseExpression.h"

class VoidExpression: public BaseExpression {
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;
    void serialize_sub(std::ostream &stream) const override {}
};


#endif // VOID_EXPRESSION_H
