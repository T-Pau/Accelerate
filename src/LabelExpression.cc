//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelExpression.h"

#include "Expression.h"
#include "ParseException.h"

Expression LabelExpression::create(const std::shared_ptr<Label>& label) {
    auto value = label->offset.size();
    if (value.has_value()) {
        return Expression(*value);
    }
    else {
        return Expression(std::make_shared<LabelExpression>(Location(), label));
    }
}


std::optional<Expression> LabelExpression::evaluated(const EvaluationContext& context) const {
    switch (type) {
        case NAMED:
            break;

        case NEXT_UNNAMED:
            context.scope->add_forward_unnamed_label_use(this);
            break;

        case PREVIOUS_UNNAMED:
            try {
                label = context.scope->get_previous_unnamed_label();
                type = NAMED;
            }
            catch (Exception &ex) {
                if (!context.conditional_in_scope) {
                    throw ex;
                }
            }
            break;
    }

    if (label) {
        auto v = label->offset.size();
        if (v.has_value()) {
            return Expression(*v);
        }
    }
    return {};
}

void LabelExpression::serialize_sub(std::ostream &stream) const {
    stream << ".label_offset(";
    switch (type) {
        case NAMED:
            stream << label->name;
            break;

        case NEXT_UNNAMED:
            stream << ":+";
            break;

        case PREVIOUS_UNNAMED:
            stream << ":-";
    }
    stream << ")";
}


std::optional<Value> LabelExpression::maximum_value() const {
    if (!label) {
        return {};
    }
    auto v = label->offset.maximum;

    if (v) {
        return Value(*v);
    }
    else {
        return {};
    }
}

std::optional<Value> LabelExpression::value() const {
    if (!label) {
        return {};
    }
    auto offset = label->offset.size();

    if (offset) {
        return Value(*offset);
    }
    else {
        return {};
    }
}

std::optional<Value> LabelExpression::minimum_value() const {
    if (!label) {
        return {};
    }
    return Value(label->offset.minimum);
}
