#ifdef IN_XLR8_BASE_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_BASE_EXPRESSION_H
#ifndef HAD_XLR8_BASE_EXPRESSION_H
#define HAD_XLR8_BASE_EXPRESSION_H

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

#include <tpau-cpp-kernal/Location.h>
#include <tpau-cpp-kernal/LocationException.h>
#include <tpau-cpp-kernal/Value.h>

#include "Base.h"

using namespace tpau::cpp_kernal;

class Entity;
class Expression;
class EvaluationContext;
class Object;
class Scope;

/**
 * Abstract base class for all expressions.
 */
class BaseExpression : public Base {
  public:
    /// @brief Initialize a BaseExpression with default values.
    BaseExpression() = default;

    /**
     * @brief Initialize a BaseExpression with a specific location.
     *
     * @param location The location of the expression.
     */
    explicit BaseExpression(const Location& location) : location(location) {}

    /**
     * @brief Destroy the BaseExpression object.
     */
    virtual ~BaseExpression() = default;

    /**
     * @brief Check if the value of the expression is known.
     *
     * The default implementation returns `true` if `value()` returns a value, `false` otherwise. If the value is expensive to compute, subclasses may override this method to provide a more efficient implementation.
     *
     * @return `true` if the value is known, `false` otherwise.
     */
    [[nodiscard]] virtual bool has_value() const { return value().has_value(); }

    /**
     * Get the value of the expression if it is known.
     *
     * Subclasses should override this method. The default implementation returns {}.
     *
     * @return The value of the expression, if it is known.
     */
    [[nodiscard]] virtual std::optional<Value> value() const { return {}; }

    /**
     * @brief Get the maximum possible value of the expression.
     *
     * The default implementation returns the value of the expression if it is known, or {} otherwise. Subclasses should override this method if they can determine a maximum value without knowing the exact value of the expression.
     *
     * @return The maximum possible value of the expression if it can be determined, {} otherwise.
     */
    [[nodiscard]] virtual std::optional<Value> minimum_value() const { return value(); }

    /**
     * @brief Get the minimum possible value of the expression.
     *
     * The default implementation returns the value of the expression if it is known, or {} otherwise. Subclasses should override this method if they can determine a minimum value without knowing the exact value of the expression.
     *
     * @return The minimum possible value of the expression. if it can be determined, {} otherwise.
     */
    [[nodiscard]] virtual std::optional<Value> maximum_value() const { return value(); }

    /**
     * @brief Get the type of the expression's value.
     *
     * The default implementation returns the type of the value of the expression if it is known, or {} otherwise. Subclasses should override this method if they can determine the type of the expression without knowing the exact value of the expression.
     *
     * @return The type of the expression's value if it can be determined, {} otherwise.
     */
    [[nodiscard]] virtual std::optional<Value::Type> type() const { return value() ? value()->type() : std::optional<Value::Type>{}; }

    /**
     * Serialize the expression to a stream.
     *
     * @param stream The stream to serialize to.
     */
    void serialize(std::ostream& stream) const;

    /**
     * Evaluate the expression in the given context.
     *
     * The default implementation calls `evaluate()` on all sub-expressions using `traverse()`, and then calls `evaluate_process()`.
     *
     * Errors during evaluation should either be reported via DiagnosticOutput and result in the expression being marked as invalid, or an exception should be thrown.
     *
     * @param context The evaluation context.
     * @return The Expression to replace this expression with, {} if no replacement is needed.
     */
    [[nodiscard]] virtual std::optional<Expression> evaluate(const EvaluationContext& context);

    /**
     * Resolve all names in the expression.
     *
     * The default implementation calls `resolve()` on all sub-expressions using `traverse()`.
     *
     * If a name cannot be resolved, either an error should be reported via DiagnosticOutput and the expression marked as invalid, or an exception should be thrown.
     *
     * @param scope The scope to resolve names in.
     * @param containing_entity The entity that contains the expression.
     */
    virtual void resolve(Scope* scope, Entity* containing_entity);

    /**
     * Expand function calls in the expression.
     *
     * The default implementation calls `expand_calls()` on all sub-expressions using `traverse()`.
     *
     * If a function call cannot be expanded, either an error should be reported via DiagnosticOutput and the expression marked as invalid, or an exception should be thrown.
     */
    virtual void expand_calls();

    /**
     * Create a deep copy of the expression.
     *
     * If a subclass has children or ever returns `true` in `needs_cloning()`, it must override this method.
     *
     * @return A shared pointer to the cloned expression.
     */
    [[nodiscard]] virtual std::shared_ptr<BaseExpression> clone() const { throw LocationException(location, "internal error: clone() not implemented for {}", typeid(*this).name()); }

    /**
     * @brief Check if the expression needs cloning.
     *
     * The default implementation returns `true`.
     *
     * If calling `evaluate()` never changes any of the expression's members, it can be reused instead of cloned.
     *
     * If any of its children need cloning, the expression will be cloned regardless of the return value of this method.
     *
     * @return `true` if the expression needs cloning, `false` if it can be reused.
     */
    [[nodiscard]] virtual bool needs_cloning() { return true; }

    /**
     * @brief Check if any of the expression's children need cloning.
     *
     * @return `true` if any child expression needs cloning, `false` otherwise.
     */
    [[nodiscard]] bool children_need_cloning();

    /**
     * The location of the expression in the source code.
     */
    Location location;

    /**
     * Indicates whether the expression is valid.
     *
     * If an error occurs during evaluation, this flag should be set to `false`. This allows the expression to be used in further evaluations without causing additional errors.
     */
    bool valid{true};

  protected:
    /**
     * Call a method on all sub-expressions of this expression.
     *
     * It is used by the default implementations of `resolve()` and `expand_calls()`. Unless a subclass has no sub-expressions or overrides both of these methods, it should override `traverse()`.
     *
     * @param callable A callable to apply to each sub-expression.
     */
    virtual void traverse(std::function<void(Expression&)> callable) {}

    /**
     * Process the evaluation of the expression.
     *
     * This method is called by the default implementation of `evaluate()` after calling `evaluate()` on all sub-expressions.
     *
     * @param context The evaluation context.
     * @return If the expression changed, returns the new expression, otherwise {}.
     */
    virtual std::optional<Expression> evaluate_process(const EvaluationContext& context);

    /**
     * Serialize the expression to a stream.
     *
     * This method must be implemented by derived classes.
     *
     * @param stream The stream to serialize to.
     */
    virtual void serialize_sub(std::ostream& stream) const = 0;
};

/**
 * Output expression to a stream.
 *
 * @param stream The stream to output to.
 * @param node Pointer to the expression to output.
 * @return The stream after outputting the expression.
 */
std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BaseExpression>& node);

/**
 * Output expression to a stream.
 *
 * @param stream The stream to output to.
 * @param node Expression to output.
 * @return The stream after outputting the expression.
 */
std::ostream& operator<<(std::ostream& stream, const BaseExpression& node);

#endif // HAD_XLR8_BASE_EXPRESSION_H
#undef IN_XLR8_BASE_EXPRESSION_H
