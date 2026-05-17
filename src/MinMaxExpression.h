#ifndef MIN_MAX_EXPRESSION_H
#define MIN_MAX_EXPRESSION_H

/*
MinMaxExpression.h --

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

#include "BaseExpression.h"
#include "Expression.h"

/// @brief Expression representing the minimum or maximum of two sub-expressions.
class MinMaxExpression: public BaseExpression {
  public:
    /** 
     * Create a minimum or maximum expression.
     * 
     * @param location The location of the expression in the source code.
     * @param a The first sub-expression.
     * @param b The second sub-expression.
     * @param minimum Whether this is a minimum expression (`true`) or a maximum expression (`false`).
     */
    explicit MinMaxExpression(const Location& location, Expression a, Expression b, bool minimum): BaseExpression(location), a{std::move(a)}, b{std::move(b)}, minimum{minimum} {}
    
    /**
     * Create a minimum expression from arguments.
     * 
     * @param location The location of the expression in the source code.
     * @param arguments The arguments of the expression. Must contain exactly two expressions.
     * @return The created expression.
     */
    static Expression create_min(const Location& location, const std::vector<Expression>& arguments) {return create(location, true, arguments);}

    /** 
     * Create a maximum expression from arguments.
     * 
     * @param location The location of the expression in the source code.
     * @param arguments The arguments of the expression. Must contain exactly two expressions.
     * @return The created expression.
     */
    static Expression create_max(const Location& location, const std::vector<Expression>& arguments) {return create(location, false, arguments);}

    /**
     * Create an expression representing the minimum or maximum of two sub-expressions.
     * 
     * This might not create a MinMaxExpression if the operation can be simplified (e.g. if both operands have values).
     * 
     * @param location The location of the expression in the source code.
     * @param a The first sub-expression.
     * @param b The second sub-expression.
     * @param minimum Whether this is a minimum expression (`true`) or a maximum expression (`false`).
     * @return The created expression.
     */
    static Expression create(const Location& location, const Expression& a, const Expression& b, bool minimum);

    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;

    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;

  protected:
    void serialize_sub(std::ostream& stream) const override;

  private:
    /**
     * Create an expression representing a minimum or maximum expression from arguments.
     * 
     * @param location The location of the expression in the source code.
     * @param minimum Whether this is a minimum expression (`true`) or a maximum expression (`false`).
     * @param arguments The arguments of the expression. Must contain exactly two expressions.
     * @return The created expression.
     */
    static Expression create(const Location& location, bool minimum, const std::vector<Expression>& arguments);

    /**
     * Get the minimum or maximum of two values.
     * 
     * @param minimum Whether to get the minimum (`true`) or maximum (`false`).
     * @param a The first value.
     * @param b The second value.
     * @return The minimum or maximum of the two values.
     */
    static Value min_max(bool minimum, const Value& a, const Value& b);

    /// @brief The first sub-expression.
    Expression a;

    /// @brief The second sub-expression.
    Expression b;

    /// @brief Whether this is a minimum expression (`true`) or a maximum expression (`false`).
    bool minimum;
};


#endif // MIN_MAX_EXPRESSION_H
