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

#include "ScopeExpression.h"

#include <tpau-cpp-kernal/LocationException.h>

Expression ScopeExpression::clone(const CloneContext& context) const {
    auto new_inner_scope = std::make_shared<Scope>(Visibility::ARGUMENT, Symbol{}, context.containing_scope);
    auto inner_context = CloneContext(new_inner_scope, &context);
    auto new_scope_expression = std::make_shared<ScopeExpression>(new_inner_scope, Expression{});

    for (auto& constant : constants) {
        auto new_constant = std::make_shared<Constant>(constant->location, constant->name, constant->visibility, new_inner_scope, false, constant->value.clone(context));
        inner_context.add_mapping(constant, new_constant);
        new_scope_expression->add(new_constant);
    }
    auto new_expression = expression.clone(inner_context);
    new_scope_expression->expression = new_expression;
    return Expression(new_scope_expression);
}

std::optional<Expression> ScopeExpression::evaluate_process(const EvaluationContext& context) {
    std::erase_if(constants, [this](auto& constant) {
        if (!constant->is_referenced()) {
            TRACE("evaluating scope", "removing unused constant {} {}", static_cast<void*>(constant.get()), constant->name);
            inner_scope()->remove_constant(constant->name);
            return true;
        }
        return false;
    });

    if (constants.empty()) {
        return expression;
    }
    return {};
}

void ScopeExpression::traverse(std::function<void(Expression&)> expression_callable) {
    for (auto constant : inner_scope()->get_constants()) {
        TRACE_BEGIN("traversing scope", "constant {} {}", static_cast<void*>(constant.get()), constant->name);
        expression_callable(constant->value);
        TRACE_END("traversing scope", "constant {}", constant->name);
    }
    TRACE_BEGIN("traversing expression", "");
    expression_callable(expression);
    TRACE_END("traversing expression", "");
}

void ScopeExpression::resolve(Scope* scope, Entity* containing_entity) {
    // Resolve the constants in the outer scope
    for (auto constant : constants) {
        TRACE_BEGIN("resolving scope", "constant {} {}", static_cast<void*>(constant.get()), constant->name);
        constant->value.resolve(scope, containing_entity);
        TRACE_END("resolving scope", "constant {}", constant->name);
    }

    // Resolve the expression in the inner scope
    TRACE_BEGIN("resolving expression", "in scope {}", static_cast<void*>(inner_scope().get()));
    expression.resolve(inner_scope().get(), containing_entity);
    TRACE_END("resolving expression", "");
}

void ScopeExpression::serialize_sub(std::ostream& stream) const {
    // TODO: Implement; also needs support in ExpressionParser.
    throw LocationException(location, "ScopeExpression::serialize_sub not implemented");
}

void ScopeExpression::add(std::shared_ptr<Constant> constant) {
    if (constant->visibility != Visibility::ARGUMENT) {
        throw Exception("internal error: cannot add constant {} with visibility {} to ScopeExpression, add to entity or module instead", constant->name, constant->visibility);
    }
    inner_scope()->add(constant);
    constants.insert(constant);
}
