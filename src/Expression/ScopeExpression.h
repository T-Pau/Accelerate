#ifdef IN_XLR8_SCOPE_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_SCOPE_EXPRESSION_H
#ifndef HAD_XLR8_SCOPE_EXPRESSION_H
#define HAD_XLR8_SCOPE_EXPRESSION_H

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

#include "Expression/BaseExpression.h"
#include "Expression/Expression.h"
#include "Scope.h"

/// @brief Represents an expression that introduces a new scope for its contained expression.
class ScopeExpression : public BaseExpression {
  public:
    /**
     * @brief Create a new ScopeExpression.
     *
     * @param containing_scope The scope containing the ScopeExpression.
     * @param expression The expression to wrap.
     * @return The ScopeExpression.
     */
    static Expression create(const std::shared_ptr<Scope>& containing_scope, Expression expression) { return Expression(std::make_shared<ScopeExpression>(std::make_shared<Scope>(Visibility::ARGUMENT, std::move(containing_scope)), std::move(expression))); }

    /**
     * @brief Construct a new ScopeExpression.
     *
     * @param containing_scope The scope containing the new scope.
     * @param expression The expression to wrap.
     */
    ScopeExpression(const std::shared_ptr<Scope>& inner_scope, Expression expression) : inner_scope_(inner_scope), expression(std::move(expression)) {}

    /**
     * @brief Get the inner scope of the ScopeExpression.
     *
     * @return The inner_scope.
     */
    [[nodiscard]] std::shared_ptr<Scope> inner_scope() const { return inner_scope_; }

    [[nodiscard]] bool has_value() const override { return expression.has_value(); }

    [[nodiscard]] std::optional<Value> value() const override { return expression.value(); }

    [[nodiscard]] std::optional<Value> maximum_value() const override { return expression.maximum_value(); }

    [[nodiscard]] std::optional<Value> minimum_value() const override { return expression.minimum_value(); }

    [[nodiscard]] std::optional<Value::Type> type() const override { return expression.type(); }

    [[nodiscard]] Expression clone(const CloneContext& context) const override;

    void add(std::shared_ptr<Constant> constant);

  protected:
    [[nodiscard]] std::optional<Expression> evaluate_process(const EvaluationContext& context) override;
    void resolve(Scope* scope, Entity* containing_entity) override;
    void serialize_sub(std::ostream& stream) const override;
    void traverse(std::function<void(Expression&)> callable) override;

  protected:
    /// @brief The scope introduced by the ScopeExpression.
    std::shared_ptr<Scope> inner_scope_;

    std::unordered_set<std::shared_ptr<Constant>> constants;

    /// @brief The expression contained within the ScopeExpression.
    Expression expression;
};

#endif // HAD_XLR8_SCOPE_EXPRESSION_H
#undef IN_XLR8_SCOPE_EXPRESSION_H
