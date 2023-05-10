//
// Created by Dieter Baron on 10.05.23.
//

#include "MemoryBodyElement.h"

uint64_t MemoryBodyElement::maximum_size() const {
    auto s = start_address.minimum_value();
    auto minimum_start = s ? s->unsigned_value() : 0;
    auto e = end_address.maximum_value();
    if (!e) {
        return std::numeric_limits<uint64_t>::max();
    }

    return e->unsigned_value() - minimum_start;
}

uint64_t MemoryBodyElement::minimum_size() const {
    auto s = start_address.maximum_value();
    auto e = end_address.minimum_value();
    if (!s || !e) {
        return 0;
    }
    return e->unsigned_value() - s->unsigned_value();
}

BodyElement::EvaluationResult MemoryBodyElement::evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const {
    auto result = EvaluationResult(minimum_offset, maximum_offset);
    auto new_start_address = start_address.evaluated(environment);
    auto new_end_address = end_address.evaluated(environment);
    if (new_start_address || new_end_address) {
        result.element = std::make_shared<MemoryBodyElement>(new_start_address.value_or(start_address), new_end_address.value_or(end_address));
    }
    result.minimum_offset += minimum_size();
    result.maximum_offset += maximum_size();

    return result;
}

void MemoryBodyElement::serialize(std::ostream &stream, const std::string &prefix) const {
    stream << prefix << ".memory " << start_address << ", " << end_address << std::endl;
}

std::optional<uint64_t> MemoryBodyElement::size() const {
    if (start_address.has_value() && end_address.has_value()) {
        return end_address.value()->unsigned_value() - start_address.value()->unsigned_value();
    }
    else {
        return {};
    }
}
