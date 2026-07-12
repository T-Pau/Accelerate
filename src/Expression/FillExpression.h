#ifdef IN_XLR8_FILL_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_FILL_EXPRESSION_H
#ifndef HAD_XLR8_FILL_EXPRESSION_H
#define HAD_XLR8_FILL_EXPRESSION_H

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

/// @brief Expression representing a sequence of repeated values: `.fill(count, value)`.
class FillExpression: public BaseExpression {
public:
    /**
     * Create a fill expression.
     * 
     * @param location The location of the expression in the source code.
     * @param count The expression representing the number of repetitions.
     * @param value The expression representing the value to repeat.
     */
    explicit FillExpression(const Location& location, Expression count, Expression value): BaseExpression(location), count{std::move(count)}, value{std::move(value)} {}

    /**
     * Create an expression representing a FillExpression from arguments.
     * 
     * This may not create a FillExpression if the expression can be simplified.
     * 
     * @param location The location of the expression in the source code.
     * @param arguments The arguments of the expression. Must contain exactly two expressions: the count and the value.
     * @return The created expression.
     * @throws LocationException If the number of arguments is not exactly two or the first argument is not an unsigned integer expression.
     */
    static Expression create(const Location& location, const std::vector<Expression>& arguments);

    /**
     * Create an expression representing a FillExpression.
     * 
     * This may not create a FillExpression if the expression can be simplified.
     * 
     * @param location The location of the expression in the source code.
     * @param count The expression representing the number of repetitions.
     * @param value The expression representing the value to repeat.
     * @return The created expression.
     * @throws LocationException If the first argument is not an unsigned integer expression.
     */
    static Expression create(const Location& location, Expression count, Expression value);

    [[nodiscard]] std::optional<Expression> evaluate(const EvaluationContext& context) override;

protected:
    void traverse(std::function<void(Expression&)> callback) override {callback(count); callback(value);}
    std::optional<Expression> evaluate_process(const EvaluationContext& context) override;
    void serialize_sub(std::ostream& stream) const override;

private:
    static std::optional<Expression> simplify(const Location& location, Expression count, Expression value, bool always_create);

    /// @brief The expression representing the number of repetitions.
    Expression count;

    /// @brief The expression representing the value to repeat.
    Expression value;
};

#endif // HAD_XLR8_FILL_EXPRESSION_H
#undef IN_XLR8_FILL_EXPRESSION_H
