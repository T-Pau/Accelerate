#include "Expression/ConstantExpression.h"

#include "Expression/ValueExpression.h"

std::optional<Expression> ConstantExpression::simplify(const Location& location, std::variant<std::shared_ptr<Constant>, Constant*> constant, bool always_create) {
    if (std::holds_alternative<std::shared_ptr<Constant>>(constant)) {
        auto argument_constant = std::get<std::shared_ptr<Constant>>(constant);

        if (argument_constant->has_value()) {
            return Expression(ValueExpression::create(location, *(argument_constant->value.value())));
        }
        else if (argument_constant->visibility != Visibility::ARGUMENT) {
            // If this is not an argument constant, relinquish ownership.
            return Expression(std::make_shared<ConstantExpression>(location, argument_constant.get()));
        }
    }
    else if (std::holds_alternative<Constant*>(constant) && std::get<Constant*>(constant)->has_value()) {
        return Expression(ValueExpression::create(location, *(std::get<Constant*>(constant)->value.value())));
    }

    if (always_create) {
        return Expression(std::make_shared<ConstantExpression>(location, constant));
    }
    else {
        return {};
    }
}

std::optional<Expression> ConstantExpression::evaluate(const EvaluationContext& context) {
    if (constant()->has_value()) {
        return Expression(ValueExpression::create(location, *(constant()->value.value())));
    }
    else if (is_argument_constant()) {
        auto argument_constant = std::get<std::shared_ptr<Constant>>(constant_);
        if (argument_constant.use_count() == 3) {
            // If this is an argument constant that is not used anywhere else, return the expression it is defined as.
            // The use_count() is 3 because the constant is held by argument_constant, constant_ variant, and ScopeBody that contains it.
            // TODO: Override location?
            TRACE("evaluating constant", "inlining argument constant {}", argument_constant->name);
            return argument_constant->value;
        }
    }
    return simplify(location, constant_, false);
}
