//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelBody.h"

#include <iostream>

#include "Body.h"
#include "Object.h"

void LabelBody::serialize(std::ostream &stream, const std::string& prefix) const {
    if (prefix.ends_with("  ")) {
        stream << prefix.substr(0, prefix.size() - 2);
    }
    else {
        stream << prefix;
    }
    stream << label->name << ":" << std::endl;
}

std::optional<Body> LabelBody::evaluated(const EvaluationContext& context) const {
    label->offset = context.offset;

    if (label->is_named()) {
        if (!context.conditional && context.object) {
            context.object->environment->add(label->name, label);
            if (label->offset.size()) {
                return Body();
            }
        }
    }
    else {
        if (context.conditional_in_scope) {
            context.scope->invalidate_unnamed_label();
        }
        else {
            context.scope->set_unnamed_label(label);
        }
    }

    return {};
}
