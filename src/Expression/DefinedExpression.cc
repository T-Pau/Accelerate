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

#include "Expression/DefinedExpression.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "EvaluationContext.h"
#include "Expression/ValueExpression.h"
#include "Expression/VariableExpression.h"
#include "Scope.h"

using namespace tpau::cpp_kernal;

Expression DefinedExpression::create(const Location& location, const std::vector<Expression>& arguments) {
    if (arguments.size() != 1) {
        throw LocationException(location, "invalid number of arguments");
    }
    auto& argument = arguments[0];

    if (!argument.is<VariableExpression>()) {
        throw LocationException(argument.location(), "symbol argument required");
    }

    return Expression(std::make_shared<DefinedExpression>(location, argument.as<VariableExpression>()->variable()));
}


void DefinedExpression::resolve(Scope* scope, Entity* containing_entity) {
    defined = scope->is_defined(symbol);
}

std::optional<Expression> DefinedExpression::evaluate(const EvaluationContext& context) {
    if (defined.has_value()) {
        return ValueExpression::create(location, Value(*defined));
    }
    return {};
}

void DefinedExpression::serialize_sub(std::ostream& stream) const {
    stream << ".defined(" << symbol << ")";
}
