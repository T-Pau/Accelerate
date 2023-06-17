/*
AssignmentBody.cc --

Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
