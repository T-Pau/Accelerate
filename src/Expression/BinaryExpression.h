#ifdef IN_XLR8_BINARY_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_BINARY_EXPRESSION_H
#ifndef HAD_XLR8_BINARY_EXPRESSION_H
#define HAD_XLR8_BINARY_EXPRESSION_H

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

/**
 * Expression representing a binary operation.
 */
class BinaryExpression : public BaseExpression {
  public:
    // clang-format off
    // Keep in sync with operation_names
    /// @brief The operations supported by the BinaryExpression class.
    enum class Operation {
        ADD,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        DIVIDE,
        EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,
        LOGICAL_AND,
        LOGICAL_OR,
        MODULO,
        MULTIPLY,
        NOT_EQUAL,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        SUBTRACT
    };
    // clang-format on

    /**
     * Create an expression from a binary operation.
     *
     * This might not create a BinaryExpression if the operation can be simplified (e. g. if both operands have values).
     *
     * @param location The location of the expression in the source code.
     * @param left The left operand.
     * @param operation The binary operation.
     * @param right The right operand.
     * @return The created expression.
     */
    [[nodiscard]] Expression static create(const Location& location, const Expression& left, Operation operation, const Expression& right) { return *simplify(location, left, operation, right, true); }

    /**
     * Construct a binary expression.
     *
     * @param location The location of the expression in the source code.
     * @param left The left operand.
     * @param operation The binary operation.
     * @param right The right operand.
     */
    BinaryExpression(const Location& location, Expression left, Operation operation, Expression right) : BaseExpression(location), left(std::move(left)), operation(operation), right(std::move(right)) {}

    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value::Type> type() const override;

    [[nodiscard]] bool needs_cloning() override { return false; }

    [[nodiscard]] Expression clone(const CloneContext& context) const override { return Expression(std::make_shared<BinaryExpression>(location, left.clone(context), operation, right.clone(context))); }

  protected:
    [[nodiscard]] std::optional<Expression> evaluate_process(const EvaluationContext& context) override;
    void serialize_sub(std::ostream& stream) const override;

    void traverse(std::function<void(Expression&)> callable) override {
        callable(left);
        callable(right);
    }

  private:
    /**
     * Simplify a binary expression if possible.
     *
     * @param location The location of the expression in the source code.
     * @param left The left operand.
     * @param operation The binary operation.
     * @param right The right operand.
     * @param always_create Whether to always create a new expression even if it cannot be simplified.
     * @return The simplified expression, or {} if no simplification is possible.
     */
    static std::optional<Expression> simplify(const Location& location, const Expression& left, Operation operation, const Expression& right, bool always_create);

    [[nodiscard]] static Entity* get_referenced_object_or_macro(const Expression& expression);

    /**
     * Get the name of a binary operation.
     *
     * @return The name of the binary operation.
     */
    [[nodiscard]] const std::string& operation_name() const { return operation_name(operation); }

    /**
     * Get the name of a binary operation.
     *
     * @param operation The binary operation.
     * @return The name of the binary operation.
     */
    [[nodiscard]] static const std::string& operation_name(Operation operation);

    /// @brief The names of the binary operations.
    static std::vector<std::string> operation_names;

    /// @brief The left operand of the binary operation.
    Expression left;

    /// @brief The binary operation.
    Operation operation;

    /// @brief The right operand of the binary operation.
    Expression right;
};


#endif // HAD_XLR8_BINARY_EXPRESSION_H
#undef IN_XLR8_BINARY_EXPRESSION_H
