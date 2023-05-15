//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelExpression.h"

#include "Expression.h"

Expression LabelExpression::create(const std::shared_ptr<Label>& label) {
    auto value = label->offset.size();
    if (value.has_value()) {
        return Expression(*value);
    }
    else {
        return Expression(std::make_shared<LabelExpression>(label));
    }
}


std::optional<Expression> LabelExpression::evaluated(const EvaluationContext& context) const {
    auto v = label->offset.size();
    if (v.has_value()) {
        return Expression(*v);
    }
    else {
        return {};
    }
}

void LabelExpression::serialize_sub(std::ostream &stream) const {
    stream << ".label_offset(" << label->name << ")";
}


std::optional<Value> LabelExpression::maximum_value() const {
    auto v = label->offset.maximum;

    if (v) {
        return Value(*v);
    }
    else {
        return {};
    }
}

std::optional<Value> LabelExpression::value() const {
    auto offset = label->offset.size();

    if (offset) {
        return Value(*offset);
    }
    else {
        return {};
    }
}
