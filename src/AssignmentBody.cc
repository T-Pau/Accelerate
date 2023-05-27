//
// Created by Dieter Baron on 27.05.23.
//

#include "AssignmentBody.h"

#include "ObjectFile.h"

std::optional<Body>
AssignmentBody::evaluated(const EvaluationContext &context) const {
    if (context.conditional_in_scope || !context.object_file) {
        return {};
    }
    if (visibility == Visibility::SCOPE) {
        context.environment->add(name, value);
    }
    else {
        // TODO: this can't be written out if label is not resolved yet.
        context.object_file->add_constant(name, visibility, value);
    }

    return Body();
}

void
AssignmentBody::serialize(std::ostream &stream, const std::string &prefix) const {
    stream << prefix;
    if (visibility != Visibility::SCOPE) {
        stream << "." << visibility << " ";
    }
    stream << name << " = " << value;
}
