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
