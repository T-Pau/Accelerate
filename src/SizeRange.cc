//
// Created by Dieter Baron on 09.05.23.
//

#include "SizeRange.h"

SizeRange SizeRange::operator+(const SizeRange &other) const {
    if (!maximum || !other.maximum) {
        return {minimum + other.minimum, {}};
    }
    else {
        return {minimum + other.minimum, *maximum + *other.maximum};
    }
}

SizeRange SizeRange::operator-(const SizeRange &other) const {
    if (!maximum || !other.maximum) {
        return {minimum - other.minimum, {}};
    }
    else {
        return {minimum - other.minimum, *maximum - *other.maximum};
    }
}

std::optional<uint64_t> SizeRange::size() const {
    if (maximum.has_value() && minimum == *maximum) {
        return minimum;
    }
    else {
        return {};
    }
}

std::ostream& operator<<(std::ostream& stream, const SizeRange& size_range) {
    stream << "(" << size_range.minimum << ",";
    if (size_range.maximum.has_value()) {
        stream << " " << *size_range.maximum;
    }
    stream << ")";

    return stream;
}
