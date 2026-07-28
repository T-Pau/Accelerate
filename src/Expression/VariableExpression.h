#ifdef IN_XLR8_VARIABLE_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_VARIABLE_EXPRESSION_H
#ifndef HAD_XLR8_VARIABLE_EXPRESSION_H
#define HAD_XLR8_VARIABLE_EXPRESSION_H

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

#include "BaseExpression.h"
#include "Expression.h"

/// @brief Expression node representing a variable.
class VariableExpression : public BaseExpression {
  public:
    /**
     * Create a variable expression.
     *
     * @param location The location of the expression.
     * @param symbol The name of the variable.
     */
    static Expression create(const Location& location, Symbol symbol) { return Expression(std::make_shared<VariableExpression>(location, symbol)); }

    /**
     * Create a variable expression.
     *
     * @param location The location of the expression.
     * @param symbol The name of the variable.
     */
    explicit VariableExpression(const Location& location, Symbol symbol, std::optional<Expression> expression = {}) : BaseExpression(location), symbol(symbol), expression(std::move(expression)) {}

    /**
     * Get the name of the variable.
     *
     * @return The name of the variable.
     */
    [[nodiscard]] Symbol variable() const { return symbol; }

    Expression clone() const override { return Expression(std::make_shared<VariableExpression>(location, symbol, expression ? std::make_optional(expression->clone()) : std::nullopt)); }

  protected:
    // needed for Expression::variable_name()
    friend class Expression;

    [[nodiscard]] std::optional<Expression> evaluate(const EvaluationContext& context) override;

    void serialize_sub(std::ostream& stream) const override { stream << symbol.str(); }

    void resolve(Scope* scope, Entity* containing_entity) override;

  private:
    /// @brief The name of the variable.
    Symbol symbol;

    /// @brief The expression representing the value of the variable.
    std::optional<Expression> expression;
};

#endif // HAD_XLR8_VARIABLE_EXPRESSION_H
#undef IN_XLR8_VARIABLE_EXPRESSION_H
