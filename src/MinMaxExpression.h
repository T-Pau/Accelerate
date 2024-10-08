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

class MinMaxExpression: public BaseExpression {
  public:
    explicit MinMaxExpression(const Location& location, Expression a, Expression b, bool minimum): BaseExpression(location), a{std::move(a)}, b{std::move(b)}, minimum{minimum} {}
    
    static Expression create_min(const Location& location, const std::vector<Expression>& arguments) {return create(location, true, arguments);}
    static Expression create_max(const Location& location, const std::vector<Expression>& arguments) {return create(location, false, arguments);}
    static Expression create(const Location& location, const Expression& a, const Expression& b, bool minimum);

    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;

    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;

  protected:
    void serialize_sub(std::ostream& stream) const override;

  private:
    static Expression create(const Location& location, bool minimum, const std::vector<Expression>& arguments);
    static Value min_max(bool minimum, const Value& a, const Value& b);

    Expression a;
    Expression b;
    bool minimum;
};


#endif // MIN_MAX_EXPRESSION_H
