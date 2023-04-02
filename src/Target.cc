//
// Created by Dieter Baron on 17.03.23.
//

#include "Target.h"

#include "TargetGetter.h"

const Target Target::empty = Target();

const Target &Target::get(Symbol name) {
    return TargetGetter::global.get(name);
}

bool Target::is_compatible_with(const Target &other) const {
    return map.is_compatible_with(other.map) && cpu->is_compatible_with(*other.cpu);
}
