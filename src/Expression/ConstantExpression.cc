#include "Expression/ConstantExpression.h"

#include "Expression/ValueExpression.h"

std::optional<Expression> ConstantExpression::simplify(const Location& location, Constant* constant, bool always_create) {
    if (constant->has_value()) {
        return Expression(ValueExpression::create(location, *(constant->value.value())));
    }
    else if (always_create) {
        return Expression(std::make_shared<ConstantExpression>(location, constant));
    }
    else {
        return {};
    }
}
