/*
BinaryExpression.h -- 

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

#ifndef BINARY_EXPRESSION_H
#define BINARY_EXPRESSION_H

#include "BaseExpression.h"
#include "Expression.h"

class BinaryExpression: public BaseExpression {
public:
    BinaryExpression(const Location& location, Expression left, Expression::BinaryOperation operation, Expression right): BaseExpression(location), left(std::move(left)), operation(operation), right(std::move(right)) {}

    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value::Type> type() const override;

    void collect_objects(std::unordered_set<Object*>& objects) const override {
        left.collect_objects(objects);
        right.collect_objects(objects);}

protected:
    [[nodiscard]] Expression static create(const Location& location, const Expression& left, Expression::BinaryOperation operation, const Expression& right);
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;

    void serialize_sub(std::ostream& stream) const override;

    friend class Expression;

private:
    Expression left;
    Expression::BinaryOperation operation;
    Expression right;
};


#endif // BINARY_EXPRESSION_H
