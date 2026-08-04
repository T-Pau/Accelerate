#ifdef IN_XLR8_IF_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_IF_BODY_H
#ifndef HAD_XLR8_IF_BODY_H
#define HAD_XLR8_IF_BODY_H

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

#include <tpau-cpp-kernal/Exception.h>

#include "Body.h"
#include "BodyElement.h"
#include "Expression/BaseExpression.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a clause in an IfBody, consisting of a condition and a body.
 *
 * If no condition is provided, the clause is considered the "else" clause and will be executed if all previous conditions are false.
 */
class IfBodyClause {
  public:
    /**
     * @brief Construct a new IfBodyClause.
     *
     * @param condition The condition for the clause. If not provided, this is the "else" clause.
     * @param body The body to execute if the condition is true.
     */
    IfBodyClause(std::optional<Expression> condition, Body body) : condition(std::move(condition)), body(std::move(body)) {}

    IfBodyClause clone(const CloneContext& context) const { return IfBodyClause(condition ? std::make_optional(condition->clone(context)) : std::nullopt, body.clone(context)); }

    /**
     * @brief Check if the clause's condition evaluates to true.
     *
     * @return `true` if the condition is known to be true, `false` otherwise.
     */
    [[nodiscard]] bool is_true() const { return !condition || (condition->has_value() && condition->value()->boolean_value()); }

    /**
     * @brief Check if the clause's condition evaluates to false.
     *
     * @return `true` if the condition is known to be false, `false` otherwise.
     */
    [[nodiscard]] bool is_false() const { return condition && condition->has_value() && !condition->value()->boolean_value(); }

    /// @brief The condition for the clause. If not provided, this is the "else" clause.
    std::optional<Expression> condition;

    /// @brief The body to execute if the condition is true.
    Body body;
};

/**
 * @brief Represents a conditional body. Parts of this body will be discarded, based on the evaluation of the conditions.
 */
class IfBody : public BodyElement {
  public:
    /**
     * @brief Construct a new IfBody.
     *
     * @param clauses The clauses for the conditional body.
     */
    explicit IfBody(std::vector<IfBodyClause> clauses);

    /**
     * @brief Create an IfBody from a list of clauses.
     *
     * This might not create an IfBody if the clauses can be simplified.
     *
     * @param clauses The clauses for the conditional body.
     * @return The created IfBody.
     */
    static Body create(const std::vector<IfBodyClause>& clauses) { return *simplify(const_cast<std::vector<IfBodyClause>&>(clauses), true); }

    [[nodiscard]] Body clone(const CloneContext& context) const override;

    void encode(std::string& bytes, const Memory* memory) override { throw Exception("unresolved if"); }

    [[nodiscard]] std::optional<Body> evaluate(const EvaluationContext& context) override;
    // TODO: We might need to implement expand_calls and resolve, depending on how we handle conditional bodies.
    void traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) override;

    void serialize(std::ostream& stream, const std::string& prefix) const override;

  private:
    /// @brief Simplify the clauses of the IfBody.
    static std::optional<Body> simplify(std::vector<IfBodyClause>& clauses, bool always_create);

    /// @brief The clauses for the conditional body.
    std::vector<IfBodyClause> clauses;
};


#endif // HAD_XLR8_IF_BODY_H
#undef IN_XLR8_IF_BODY_H
