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

#include "VariableExpression.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "ConstantExpression.h"
#include "EvaluationContext.h"
#include "ObjectExpression.h"
#include "Scope.h"
#include "ValueExpression.h"

using namespace tpau::cpp_kernal;

std::optional<Expression> VariableExpression::evaluate(const EvaluationContext& context) {
    if (!expression) {
        // throw LocationException(location, "internal error: expression not set for variable {}", symbol);
        return {};
    }

    expression->evaluate(context);

    if (expression->has_value()) {
        return ValueExpression::create(location, *expression->value());
    }
    else {
        return expression;
    }
}

void VariableExpression::resolve(Scope* scope, Entity* containing_entity) {
    if (symbol == Token::colon_minus.as_symbol()) {
        expression = scope->get_previous_unnamed_label(location);
        if (!expression) {
            throw LocationException(location, "no previous unnamed label");
        }
    }
    else if (symbol == Token::colon_plus.as_symbol()) {
        expression = scope->get_next_unnamed_label(location);
        if (!expression) {
            throw LocationException(location, "no next unnamed label");
        }
    }
    else {
        if (auto constant = scope->get_constant(symbol)) {
            expression = ConstantExpression::create(location, constant);
            if (containing_entity) {
                containing_entity->uses(constant);
            }
        }
        else if (auto object = scope->get_object(symbol)) {
            expression = ObjectExpression::create(location, object);
            if (containing_entity) {
                containing_entity->uses(object);
            }
        }
        else {
            throw LocationException(location, "undefined variable {}", symbol);
        }
    }

    if (!expression) {
        throw LocationException(location, "internal error: expression not set after resolving {}", symbol);
    }
}
