//
// Created by Dieter Baron on 31.05.23.
//

#include "MacroBody.h"

#include "Body.h"

std::optional<Body> MacroBody::evaluated(const EvaluationContext& context) const {
    // TODO: implement
    return {};
}


void MacroBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << name.as_string();
    auto first = true;
    for (auto& argument: arguments) {
        if (first) {
            first = false;
        }
        else {
            stream << ",";
        }
        stream << " " << argument;
    }
    stream << std::endl;
}
