//
// Created by Dieter Baron on 10.05.23.
//

#include "MemoryBodyElement.h"

MemoryBodyElement::MemoryBodyElement(Expression bank_, Expression start_address_, Expression end_address_): bank(std::move(bank_)), start_address(std::move(start_address_)), end_address(std::move(end_address_)) {
    auto minimum_start = start_address.minimum_value().value_or(Value(uint64_t(0)));
    auto maximum_end = end_address.maximum_value();
    if (!maximum_end) {
        size_range_.maximum = maximum_end->unsigned_value() - minimum_start.unsigned_value();
    }

    auto maximum_start = start_address.maximum_value();
    auto minimum_end = end_address.minimum_value();
    if (maximum_start && minimum_end) {
        size_range_.minimum = maximum_start->unsigned_value() - minimum_end->unsigned_value();
    }
}

std::optional<Body> MemoryBodyElement::evaluated(const Environment &environment, const SizeRange& offset) const {
    auto new_bank = bank.evaluated(environment);
    auto new_start_address = start_address.evaluated(environment);
    auto new_end_address = end_address.evaluated(environment);
    if (new_bank || new_start_address || new_end_address) {
        return Body(std::make_shared<MemoryBodyElement>(new_bank.value_or(bank), new_start_address.value_or(start_address), new_end_address.value_or(end_address)));
    }
    else {
        return {};
    }
}

void MemoryBodyElement::serialize(std::ostream &stream, const std::string &prefix) const {
    stream << prefix << ".memory " << start_address << ", " << end_address << std::endl;
}


void MemoryBodyElement::encode(std::string &bytes, const Memory *memory) const {
    if (memory == nullptr) {
        throw Exception("can't encode .memory without memory");
    }
    if (!bank.has_value() || !start_address.has_value() || !end_address.has_value()) {
        throw Exception("unresolved address"); // TODO: more details
    }

    auto bank_value = bank.value()->unsigned_value();
    auto start_value = start_address.value()->unsigned_value();
    auto end_value = end_address.value()->unsigned_value();

    bytes += (*memory)[bank_value].data(Range(start_value, end_value - start_value));
}
