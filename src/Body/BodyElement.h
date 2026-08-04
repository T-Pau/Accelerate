#ifdef IN_XLR8_BODY_ELEMENT_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_BODY_ELEMENT_H
#ifndef HAD_XLR8_BODY_ELEMENT_H
#define HAD_XLR8_BODY_ELEMENT_H

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

#include <optional>

#include "Base.h"
#include "CloneContext.h"
#include "EvaluationContext.h"
#include "Memory.h"
#include "SizeRange.h"

class Body;
class CPU;

/**
 * Abstract base class for all Body types.
 */
class BodyElement : public Base {
  public:
    /**
     * @brief Construct a BodyElement with unknown location and size.
     */
    BodyElement() = default;

    /**
     * @brief Construct a BodyElement with a specified size range.
     *
     * @param size_range The size range of the BodyElement.
     */
    explicit BodyElement(const Location& location, const SizeRange& size_range = {}) : location(location), size_range_(size_range) {}

    /**
     * @brief Destroy the BodyElement.
     */
    virtual ~BodyElement() = default;

    /**
     * @brief Get the size range of the BodyElement.
     *
     * @return The size range of the BodyElement.
     */
    [[nodiscard]] virtual SizeRange size_range() const { return size_range_; }

    /**
     * @brief Get the offset of the BodyElement within its entity.
     *
     * @return The offset of the BodyElement.
     */
    [[nodiscard]] virtual SizeRange offset() const { return offset_; }

    /**
     * @brief Get the size of the BodyElement, if it is known.
     *
     * @return The size of the BodyElement, {} if it is unknown.
     */
    [[nodiscard]] std::optional<uint64_t> size() const { return size_range().size(); }

    [[nodiscard]] virtual Body clone(const CloneContext& context) const = 0;

    /**
     * @brief Encode the BodyElement into a byte string.
     *
     * The default implementation does nothing. Subclasses that represent data or instructions should override this method.
     *
     * @param bytes The byte string to encode into.
     * @param memory The memory context for encoding.
     */
    virtual void encode(std::string& bytes, const Memory* memory) {}

    /**
     * @brief Traverse the BodyElement.
     *
     * This function is used by the default implementations of `resolve()` and `expand_calls()`. Unless a subclass overrides both of these methods, it should override `traverse()`.
     *
     * @param body_callable A callable to apply to each sub-body-element.
     * @param expression_callable A callable to apply to each sub-expression.
     */
    virtual void traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {}

    /**
     * @brief Enter names defined by the BodyElement in the given scope.
     *
     * This method is used to enter names into the given scope.
     *
     * The default implementation calls `enter_names()` on all sub-body-elements using `traverse()`. Subclasses that define names should override this method.
     *
     * @param scope The scope to enter names into.
     * @param containing_entity The entity that contains the BodyElement.
     */
    virtual void enter_names(Scope* scope, Entity* containing_entity);

    /**
     * @brief Resolve all names in the BodyElement.
     *
     * If a name cannot be resolved, an error should be reported via DiagnosticOutput and the BodyElement marked as invalid, or an exception should be thrown.
     *
     * The default implementation calls `resolve()` on all sub-body-elements and sub-expressions using `traverse()`.
     *
     * @param scope The scope to resolve names in.
     * @param containing_entity The entity that contains the BodyElement.
     */
    virtual void resolve(Scope* scope, Entity* containing_entity);

    /**
     * @brief Expand macro and function calls in the BodyElement.
     *
     * The default implementation calls `expand_calls()` on all sub-body-elements and sub-expressions using `traverse()`.
     *
     * If a macro or function call cannot be expanded, either an error should be reported via DiagnosticOutput and the BodyElement marked as invalid, or an exception should be thrown.
     *
     * @return The Body to replace this BodyElement with, {} if no replacement is needed.
     */
    virtual std::optional<Body> expand_calls();

    /**
     * @brief Evaluate the BodyElement in a given context.
     *
     * Subclasses must implement this method.
     *
     * If an error occurs during evaluation, either an error should be reported via DiagnosticOutput and the BodyElement marked as invalid, or an exception should be thrown.
     *
     * @param context The evaluation context.
     * @return The Body to replace this BodyElement with, {} if no replacement is needed.
     */
    [[nodiscard]] virtual std::optional<Body> evaluate(const EvaluationContext& context);

    /**
     * @brief Evaluate the body element after all sub-body-elements and sub-expressions have been evaluated.
     *
     * The default implementation does nothing.
     *
     * @param context The evaluation context.
     * @return The Body to replace this BodyElement with, {} if no replacement is needed.
     */
    [[nodiscard]] virtual std::optional<Body> evaluate_process(const EvaluationContext& context);

    /**
     * @brief Check if the body and all sub-body-elements and sub-expressions have been fully evaluated.
     *
     * The default implementation uses `traverse()` to check all sub-body-elements and sub-expressions and returns `true` if they are all fully evaluated.
     *
     * @return `true` if the BodyElement is fully evaluated, `false` otherwise.
     */
    [[nodiscard]] virtual bool fully_evaluated();

    /**
     * @brief Serialize the BodyElement to a stream.
     *
     * Subclasses must implement this method.
     *
     * @param stream The stream to serialize to.
     * @param prefix The prefix for each line of the serialized output.
     */
    virtual void serialize(std::ostream& stream, const std::string& prefix) const = 0;

    /**
     * @brief Append a BodyElement to this body directly, if supported.
     *
     * If appending is supported, this method returns the resulting body. Otherwise, it returns an empty optional.
     *
     * The default implementation does not support appending. If a subclass supports appending, it should override this method.
     *
     * @param body The body to append to.
     * @param element The element to append.
     * @return The resulting body, or {} if not supported.
     */
    [[nodiscard]] virtual std::pair<bool, std::optional<Body>> append_sub(const Body& body, const Body& element);

    friend class Body;

    /// @brief The location of the BodyElement in the source code.
    Location location;

    /**
     * @brief Indicates whether the BodyElement is valid.
     *
     * If an error occurs during evaluation, this flag should be set to `false`. This allows the BodyElement to be used in further evaluations without causing additional errors.
     */
    bool valid{true};

  protected:
    /// The size range of the BodyElement.
    SizeRange size_range_ = SizeRange(0);

    /// The offset of the BodyElement within its entity.
    SizeRange offset_ = SizeRange(0, {});

  private:
    class NotFullyEvaluatedException : public std::exception {
      public:
        const char* what() const noexcept override { return "BodyElement is not fully evaluated"; }
    };
};

/**
 * @brief Print BodyElement to an output stream.
 *
 * @param stream The output stream.
 * @param element The BodyElement to print.
 * @return The output stream.
 */
std::ostream& operator<<(std::ostream& stream, const BodyElement& element);

/**
 * @brief Print the BodyElement pointed to by a shared pointer to an output stream.
 *
 * @param stream The output stream.
 * @param element The shared pointer to the BodyElement to print.
 * @return The output stream.
 */
std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BodyElement>& element);

#endif // HAD_XLR8_BODY_ELEMENT_H
#undef IN_XLR8_BODY_ELEMENT_H
