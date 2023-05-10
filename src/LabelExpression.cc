//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelExpression.h"

#include "Expression.h"

Expression LabelExpression::create(const std::shared_ptr<LabelBodyElement>& label) {
    auto value = label->value();
    if (value.has_value()) {
        return Expression(*value);
    }
    else {
        return Expression(std::make_shared<LabelExpression>(label));
    }
}


std::optional<Expression> LabelExpression::evaluated(const Environment &environment) const {
    auto v = label->value();
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
