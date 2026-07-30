#include "Expression/ConstantExpression.h"

#include "Expression/ValueExpression.h"

ConstantExpression::~ConstantExpression() {
    auto constant = constant_.lock();
    if (constant) {
        constant->remove_reference();
    }
}

std::optional<Expression> ConstantExpression::simplify(const Location& location, std::shared_ptr<Constant> constant, bool always_create) {
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

std::optional<Expression> ConstantExpression::evaluate(const EvaluationContext& context) {
    if (constant()->has_value()) {
        return Expression(ValueExpression::create(location, *(constant()->value.value())));
    }
    else if (is_argument_constant()) {
        auto argument_constant = constant();
        if (argument_constant->single_reference()) {
            // If this is an argument constant that is not used anywhere else, return the expression it is defined as.
            TRACE("evaluating constant", "inlining argument constant {}", argument_constant->name);
            return argument_constant->value;
        }
    }
    return {};
}

Expression ConstantExpression::clone(const CloneContext& context) const {
    auto original_constant = constant();
    auto new_constant = context.map(constant());
#ifdef TRACE_TRANSLATION
    if (new_constant != original_constant) {
        TRACE("cloning constant", "mapping {} {} to {} {}", static_cast<void*>(original_constant.get()), original_constant->name, static_cast<void*>(new_constant.get()), new_constant->name);
    }
    else {
        TRACE("cloning constant", "not mapping {} {}", static_cast<void*>(original_constant.get()), original_constant->name);
    }
#endif
    return Expression(std::make_shared<ConstantExpression>(location, new_constant));
}
