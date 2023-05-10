//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelBodyElement.h"

std::optional<Value> LabelBodyElement::value() const {
    if (offset.minimum == offset.maximum) {
        return Value(offset.minimum);
    }
    else {
        return {};
    }
}

void LabelBodyElement::serialize(std::ostream &stream, const std::string& prefix) const {
    if (prefix.ends_with("  ")) {
        stream << prefix.substr(0, prefix.size() - 2);
    }
    else {
        stream << prefix;
    }
    stream << name << ":" << std::endl;
}

BodyElement::EvaluationResult LabelBodyElement::evaluate(const Environment &environment, uint64_t new_minimum_offset, uint64_t new_maximum_offset) const {
    offset.minimum = new_minimum_offset;
    offset.maximum = new_maximum_offset;

    return {{}, new_minimum_offset, new_maximum_offset};
}

std::optional<Value> LabelBodyElement::maximum_value() const {
    if (offset.maximum) {
        return Value(*offset.maximum);
    }
    else {
        return {};
    }
}
