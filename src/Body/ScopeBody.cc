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

#include "ScopeBody.h"

#include "Entity/Constant.h"
#include "Scope.h"

ScopeBody::ScopeBody(const std::shared_ptr<Scope>& scope, Body body) : inner_scope_(std::move(scope)), body(std::move(body)) {
    if (!inner_scope_) {
        throw Exception("ScopeBody must be initialized with a non-null scope.");
    }
    if (inner_scope_->type() != Visibility::SCOPE) {
        throw Exception("ScopeBody must be initialized with a scope of type SCOPE.");
    }
}

void ScopeBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".scope {" << std::endl;
    auto inner_prefix = prefix + "  ";
    for (auto constant : inner_scope()->get_constants()) {
        stream << inner_prefix << constant->name << " = " << constant->value << std::endl;
    }
    body.serialize(stream, inner_prefix);
    stream << prefix << "}" << std::endl;
}

void ScopeBody::traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    for (auto constant : inner_scope()->get_constants()) {
        TRACE_BEGIN("traversing scope", "constant {}", constant->name);
        expression_callable(constant->value);
        TRACE_END("traversing scope", "constant {}", constant->name);
    }
    TRACE_BEGIN("traversing body", "");
    body_callable(body);
    TRACE_END("traversing body", "");
}

std::optional<Body> ScopeBody::evaluate_process(const EvaluationContext& context) {
    if (scope_fully_evaluated()) {
        return body;
    }
    return {};
}

void ScopeBody::resolve(Scope* scope, Entity* containing_entity) {
    // Resolve the constants in the outer scope
    for (auto constant : inner_scope()->get_constants()) {
        TRACE_BEGIN("resolving scope", "constant {}", constant->name);
        constant->value.resolve(scope, containing_entity);
        TRACE_END("resolving scope", "constant {}", constant->name);
    }

    // Resolve the body in the inner scope
    TRACE_BEGIN("resolving body", "");
    body.resolve(inner_scope().get(), containing_entity);
    TRACE_END("resolving body", "");
}

bool ScopeBody::scope_fully_evaluated() {
    auto constants = inner_scope()->get_constants();
    return std::all_of(constants.begin(), constants.end(), [](const auto& constant) { return !constant->is_referenced(); });
}
