//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelExpression.h"
#include "ValueExpression.h"

std::shared_ptr<Expression> LabelExpression::create(const std::shared_ptr<LabelBodyElement>& label) {
    auto value = label->value();
    if (value.has_value()) {
        return std::make_shared<ValueExpression>(*value);
    }
    else {
        return std::make_shared<LabelExpression>(label);
    }
}


std::shared_ptr<Expression> LabelExpression::evaluate(const Environment &environment) const {
    auto v = label->value();
    if (v.has_value()) {
        return std::make_shared<ValueExpression>(*v);
    }
    else {
        return {};
    }
}

void LabelExpression::serialize_sub(std::ostream &stream) const {
    stream << label->name;
}
