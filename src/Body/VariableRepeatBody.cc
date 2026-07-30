/*
Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "Body/VariableRepeatBody.h"

#include "Body/BlockBody.h"
#include "Body/SimpleRepeatBody.h"
#include "Expression/ArgumentExpression.h"
#include "Expression/ValueExpression.h"
#include "Scope.h"

VariableRepeatBody::VariableRepeatBody(std::shared_ptr<Scope> inner_scope, Location variable_location, Symbol variable, RepeatRange range, Body body) : RepeatBody(std::move(range), std::move(body)), variable{variable}, variable_location{variable_location}, inner_scope{std::move(inner_scope)} {
    variable_constant = std::make_shared<Constant>(variable_location, variable, Visibility::ARGUMENT, inner_scope, false, ArgumentExpression::create({}, variable));
    variable_constant->add_reference();
}

Body VariableRepeatBody::create(std::shared_ptr<Scope> containing_scope, Location variable_location, Symbol variable, RepeatRange range, Body body) {
    auto repeat_body = std::make_shared<VariableRepeatBody>(std::make_shared<Scope>(Visibility::ARGUMENT, containing_scope), variable_location, variable, std::move(range), std::move(body));
    if (range.start.has_value() && range.end.has_value()) {
        return repeat_body->expand();
    }
    else {
        return Body(repeat_body);
    }
}

std::optional<Body> VariableRepeatBody::evaluate(const EvaluationContext& context) {
    range.start.evaluate(context);
    range.end.evaluate(context);
    body.evaluate(context);

    if (!variable_constant->is_referenced()) {
        return SimpleRepeatBody::create(range, body);
    }

    if (range.start.has_value() && range.end.has_value()) {
        return expand();
    }
    else {
        return {};
    }
}

void VariableRepeatBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".repeat ";
    stream << variable << ", " << range.start << ", " << range.end << " {" << std::endl;
    body.serialize(stream, prefix + "  ");
    stream << "}" << std::endl;
}

void VariableRepeatBody::resolve(Scope* scope, Entity* containing_entity) {
    range.start.resolve(scope, containing_entity);
    range.end.resolve(scope, containing_entity);
    body.resolve(inner_scope.get(), containing_entity);
}

Body VariableRepeatBody::expand() {
    std::vector<Body> bodies;

    auto start_value = *range.start.value();
    auto end_value = *range.end.value();

    for (auto i = start_value; i < end_value; i += Value(uint64_t{1})) {
        auto i_constant = std::make_shared<Constant>(variable_location, variable, Visibility::ARGUMENT, inner_scope, false, ValueExpression::create(variable_location, i));
        auto context = CloneContext(inner_scope->parent());
        context.add_mapping(variable_constant, i_constant);
        bodies.push_back(body.clone(context));
    }
    return BlockBody::create(std::move(bodies));
}

Body VariableRepeatBody::clone(const CloneContext& context) const {
    // TODO: implement
    throw Exception("can't clone unresolved repeat");
}
