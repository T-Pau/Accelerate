//
// Created by Dieter Baron on 31.05.23.
//

#include "MacroBody.h"

#include "Body.h"
#include "Macro.h"

std::optional<Body> MacroBody::evaluated(const EvaluationContext& context) const {
    std::vector<Expression> new_arguments;
    auto changed = false;
    for (auto& argument: arguments) {
        auto new_argument = argument.evaluated(context);
        if (new_argument) {
            new_arguments.emplace_back(*new_argument);
            changed = true;
        }
        else {
            new_arguments.emplace_back(argument);
        }
    }

    auto macro = context.environment->get_macro(name.as_symbol());
    if (!macro) {
        if (changed) {
            return Body(name, new_arguments);
        }
        else {
            return {};
        }
    }

    return macro->expand(new_arguments);
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
