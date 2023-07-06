/*
LabelBody.cc --

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
    stream << name << ":" << std::endl;
}

std::optional<Body> LabelBody::evaluated(const EvaluationContext& context) const {
    auto new_added_to_environment = added_to_environment;
    auto new_unnamed_index = unnamed_index;

    if (context.entity) {
        if (!added_to_environment) {
            if (name.empty()) {
                new_unnamed_index = context.environment->unnamed_labels.add_label(context.offset);
            }
            else {
                context.environment->add(name, context.offset);
            }
            new_added_to_environment = true;
        }
        else if (offset != context.offset) {
            if (name.empty()) {
                context.environment->unnamed_labels.update_label(unnamed_index, context.offset);
            }
            else {
                context.environment->update(name, context.offset);
            }
        }
    }

    if (new_added_to_environment != added_to_environment || context.offset != offset) {
        return Body(name, context.offset, new_added_to_environment, new_unnamed_index);
    }
    else {
        return {};
    }
}
