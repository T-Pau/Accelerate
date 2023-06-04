//
// Created by Dieter Baron on 27.05.23.
//

#include "AssignmentBody.h"

#include "ObjectFile.h"

std::optional<Body>
AssignmentBody::evaluated(const EvaluationContext &context) const {
    if (context.conditional_in_scope || !context.object) {
        return {};
    }
    if (visibility == Visibility::SCOPE) {
        context.environment->add(name, value);
    }
    else {
        // TODO: this can't be written out if label is not resolved yet.
        context.object->owner->add_constant(std::make_unique<ObjectFile::Constant>(Token(Token::NAME, {}, name), visibility, value));
    }

    return Body();
}

void
AssignmentBody::serialize(std::ostream &stream, const std::string &prefix) const {
    stream << prefix;
    if (visibility != Visibility::SCOPE) {
        stream << "." << visibility << " ";
    }
    stream << name << " = " << value << std::endl;
}
