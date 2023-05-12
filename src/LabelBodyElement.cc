//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelBodyElement.h"

#include <iostream>

#include "Body.h"


void LabelBodyElement::serialize(std::ostream &stream, const std::string& prefix) const {
    if (prefix.ends_with("  ")) {
        stream << prefix.substr(0, prefix.size() - 2);
    }
    else {
        stream << prefix;
    }
    stream << label->name << ":" << std::endl;
}

std::optional<Body> LabelBodyElement::evaluated(const Environment &environment, const SizeRange& new_offset) const {
    label->offset = new_offset;

    if (label->offset.size().has_value()) {
        return Body();
    }
    else {
        return {};
    }
}
