//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelBodyElement.h"

std::optional<Value> LabelBodyElement::value() const {
    if (minimum_offset == maximum_offset) {
        return Value(minimum_offset);
    }
    else {
        return {};
    }
}

void LabelBodyElement::serialize(std::ostream &stream) const {
    stream << name << ":" << std::endl;
}

BodyElement::EvaluationResult LabelBodyElement::evaluate(const Environment &environment, uint64_t new_minimum_offset, uint64_t new_maximum_offset) const {
    minimum_offset = new_minimum_offset;
    maximum_offset = new_maximum_offset;

    return {{}, minimum_offset, maximum_offset};
}
