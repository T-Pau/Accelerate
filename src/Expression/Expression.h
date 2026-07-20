#ifdef IN_XLR8_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_EXPRESSION_H
#ifndef HAD_XLR8_EXPRESSION_H
#define HAD_XLR8_EXPRESSION_H

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

#include "Expression/BaseExpression.h"
#include "Tokenizer.h"

class Entity;
class EvaluationContext;
class Object;

/**
 * @brief Represents an expression. This is the class other parts of the program interact with. It is a wrapper around the BaseExpression hierarchy.
 */
class Expression {
  public:
    static Expression create(const Token& token);

    /// @brief Construct a void Expression.
    Expression();

    /**
     * @brief Construct an Expression from a BaseExpression.
     *
     * @param expression The BaseExpression to wrap.
     */
    explicit Expression(std::shared_ptr<BaseExpression> expression);

    /**
     * @brief Construct an Expression by parsing it from a Tokenizer.
     *
     * @param tokenizer The Tokenizer to use for parsing the expression.
     */
    explicit Expression(Tokenizer& tokenizer);

    /**
     * Check if the expression is of a specific type.
     *
     * @tparam T The type to check against.
     * @return `true` if the expression is of type T, `false` otherwise.
     */
    template <typename T> [[nodiscard]] bool is() const { return as<T>() != nullptr; }

    /**
     * Get the expression as a specific type.
     *
     * @tparam T The type to cast to.
     * @return A pointer to the expression as type T, or nullptr if it is not of that type.
     */
    template <typename T> [[nodiscard]] const T* as() const { return dynamic_cast<const T*>(expression.get()); }

    /**
     * @brief Evaluate the expression in a given context.
     *
     * @param context The evaluation context.
     */
    void evaluate(const EvaluationContext& context);

    /**
     * @brief Check if the value of the expression is known.
     *
     * @return `true` if the value is known, `false` otherwise.
     */
    [[nodiscard]] bool has_value() const { return value().has_value(); }

    /**
     * @brief Get the location in the source code where the expression was defined.
     */
    [[nodiscard]] const Location& location() const { return expression->location; }

    /**
     * @brief Get the maximum possible value of the expression.
     *
     * @return The maximum possible value of the expression if it can be determined, {} otherwise.
     */
    [[nodiscard]] std::optional<Value> maximum_value() const { return expression->maximum_value(); }

    /**
     * @brief Get the minimum possible value of the expression.
     *
     * @return The minimum possible value of the expression. if it can be determined, {} otherwise.
     */
    [[nodiscard]] std::optional<Value> minimum_value() const { return expression->minimum_value(); }

    /**
     * @brief Serialize the expression to a stream.
     *
     * @param stream The output stream to serialize the expression to.
     */
    void serialize(std::ostream& stream) const;

    /**
     * @brief Get the type of the expression's value.
     *
     * @return The type of the expression's value if it can be determined, {} otherwise.
     */
    [[nodiscard]] std::optional<Value::Type> type() const { return expression->type(); }

    /**
     * @brief Check if the expression has a specific type.
     *
     * If `type` is an aggregate type (like `Value::INTEGER`), this method will return `true` if the expression's type is a subtype of it.
     *
     * @param type The type to check against.
     * @return `true` if the expression has the specified type, `false` if it has a different type, and {} if the type cannot be determined.
     */
    [[nodiscard]] std::optional<bool> has_type(Value::Type type) const;

    /**
     * @brief Get the value of the expression.
     *
     * @return The value of the expression if it is known, {} otherwise.
     */
    [[nodiscard]] std::optional<Value> value() const { return expression->value(); }

    /**
     * @brief If it is a variable expression, return the variable name.
     *
     * @return The variable name, or the empty Symbol if it isn't a variable expression.
     */
    [[nodiscard]] Symbol variable_name() const;

    /**
     * @brief Create a deep copy of the expression.
     *
     * If the expression does not change during evaluation, it may be reused instead of cloned.
     *
     * @return A new Expression that is a deep copy of this one.
     */
    [[nodiscard]] Expression clone() const;

    /**
     * @brief Check if the expression needs cloning.
     *
     * If calling `evaluate()` never changes any members of the expression, and none of its children need cloning, it can be reused instead of cloned.
     *
     * @return `true` if the expression needs cloning, `false` if it can be reused.
     */
    [[nodiscard]] bool needs_cloning() { return expression->needs_cloning() || expression->children_need_cloning(); }

    /**
     * @brief Resolve all names in the expression.
     *
     * @param scope The scope to resolve names in.
     * @param containing_entity The entity that contains the expression.
     */
    void resolve(Scope* scope, Entity* containing_entity);

    /**
     * @brief Expand function calls in the expression.
     *
     * If a function call cannot be expanded, either an error should be reported via DiagnosticOutput and the expression marked as invalid, or an exception should be thrown.
     */
    void expand_calls();

    /**
     * @brief Check if the expression is valid.
     *
     * If a translation error occurred while translating the expression, it is considered invalid. This allows the expression to be used in further evaluations without causing additional errors.
     *
     * @return True if the expression is valid, false otherwise.
     */
    [[nodiscard]] bool valid() const { return expression->valid; }

  private:
    /// @brief A shared pointer to the BaseExpression that this Expression wraps.
    std::shared_ptr<BaseExpression> expression;

    /// @brief A shared pointer to a void expression, used for constructing void expressions.
    static std::shared_ptr<BaseExpression> void_expression;
};

/**
 * @brief Output the expression to a stream.
 *
 * @param stream The output stream to write to.
 * @param expression The expression to output.
 * @return The output stream.
 */
std::ostream& operator<<(std::ostream& stream, const Expression& expression);

#endif // HAD_XLR8_EXPRESSION_H
#undef IN_XLR8_EXPRESSION_H
