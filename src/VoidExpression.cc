//
// Created by Dieter Baron on 10.05.23.
//

#include "VoidExpression.h"

#include "Expression.h"

std::optional<Expression> VoidExpression::evaluated(const EvaluationContext &context) const {
    return {};
}
