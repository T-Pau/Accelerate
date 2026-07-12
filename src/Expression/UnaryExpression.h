#ifndef HAD_XLR8_UNARY_EXPRESSION_H
#define HAD_XLR8_UNARY_EXPRESSION_H

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

/// @brief Expression node representing a unary operation.
class UnaryExpression: public BaseExpression {
public:
    enum Operation {
        BANK_BYTE,
        BITWISE_NOT,
        HIGH_BYTE,
        LOW_BYTE,
        MINUS,
        NOT,
        PLUS
    };

    /**
     * Create an expression from a unary operation.
     * 
     * This might not create a UnaryExpression if the operation can be simplified (e.g. if the operand has a value).
     * 
     * @param location The location of the expression in the source code.
     * @param operation The unary operation.
     * @param operand The operand of the unary operation.
     * @return The created expression.
     */
    [[nodiscard]] static Expression create(const Location& location, Operation operation, Expression operand);

    /**
     * Create a unary expression.
     * 
     * @param location The location of the expression in the source code.
     * @param operation The unary operation.
     * @param operand The operand of the unary operation.
     */
    UnaryExpression(const Location& location, Operation operation, Expression operand): BaseExpression(location), operation(operation), operand(std::move(operand)) {}

protected:
    [[nodiscard]] std::optional<Expression> evaluate_process(const EvaluationContext& context) override;
    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value::Type> type() const override;

    void serialize_sub(std::ostream& stream) const override;
    void traverse(std::function<void(Expression&)> callable) override {callable(operand);}

private:
    static std::optional<Expression> simplify(const Location& location, Operation operation, const Expression& operand, bool always_create);

    /// @brief The unary operation.
    Operation operation;

    /// @brief The operand of the unary operation.
    Expression operand;
};


#endif // HAD_XLR8_UNARY_EXPRESSION_H
