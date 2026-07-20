/*
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

#include "Body/Body.h"
#include "Entity/Constant.h"
#include "Expression/BinaryExpression.h"
#include "Expression/LabelOffsetExpression.h"
#include "Expression/VariableExpression.h"
#include "Scope.h"

void LabelBody::serialize(std::ostream& stream, const std::string& prefix) const {
    if (prefix.ends_with("  ")) {
        stream << prefix.substr(0, prefix.size() - 2);
    }
    else {
        stream << prefix;
    }
    stream << name << ":" << std::endl;
}

std::optional<Body> LabelBody::evaluate(const EvaluationContext& context) {
#if 0
    auto new_added_to_environment = added_to_environment;
    auto new_name = name;
    auto new_unnamed_index = unnamed_index;

    if (name.empty()) {
        new_name = unnamed_label_name(context.result.next_unnamed_label++);
    }

    if (context.entity || context.type == EvaluationContext::OUTPUT) {
        if (!added_to_environment) {
            context.environment->add(name, context.offset - context.label_offset);
            new_added_to_environment = true;
        }
        else if (offset != context.offset) {
            context.environment->update(name, context.offset - context.label_offset);
        }
    }

    if (new_added_to_environment != added_to_environment || new_name != name || context.offset != offset) {
        return Body(new_name, context.offset, new_added_to_environment, new_unnamed_index);
    }
    else {
        return {};
    }
#else
    return {};
#endif
}

void LabelBody::resolve(Scope* scope, Entity* containing_entity) {
    // TODO: resolve unnamed label
}

void LabelBody::enter_names(Scope* scope, Entity* containing_entity) {
    auto offset_expression = LabelOffsetExpression::create(location, containing_entity->name, name, this);
    auto label_expression = BinaryExpression::create(location, VariableExpression::create(location, containing_entity->name), BinaryExpression::Operation::ADD, offset_expression);

    if (!name.empty()) {
        scope->add(std::make_unique<Constant>(location, name, visibility, containing_entity->get_scope(), false, label_expression));
    }
    else {
        scope->add_unnamed_label(location, label_expression);
    }
}
