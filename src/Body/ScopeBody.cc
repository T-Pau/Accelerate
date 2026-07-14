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

ScopeBody::ScopeBody(const std::shared_ptr<Scope>& scope, Body body): scope_(std::move(scope)), body(std::move(body)) {
    if (scope_->type() != Visibility::SCOPE) {
        throw Exception("ScopeBody must be initialized with a scope of type SCOPE.");
    }
}


void ScopeBody::serialize(std::ostream& stream, const std::string& prefix) const {
    // TODO: serialize environment
    stream << prefix << ".scope {" << std::endl;
    body.serialize(stream, prefix + "  ");
    stream << prefix << "}" << std::endl;
}

void ScopeBody::traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    for (auto constant: scope()->get_constants()) {
        expression_callable(constant->value);
    }
    body_callable(body);
}

std::optional<Body> ScopeBody::evaluate_process(const EvaluationContext& context) {
    if (body.fully_evaluated()) {
        return body;
    }
    return {};
}
