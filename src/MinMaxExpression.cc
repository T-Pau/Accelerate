/*
MinMaxExpression.cc --

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

#include "MinMaxExpression.h"

#include "ParseException.h"

Expression MinMaxExpression::create(const Location& location, bool minimum, const std::vector<Expression>& arguments) {
    if (arguments.size() != 2) {
        throw ParseException(location, "invalid number of arguments");
    }
    return create(location, arguments[0], arguments[1], minimum);
}

Expression MinMaxExpression::create(const Location& location, const Expression& a, const Expression& b, bool minimum) {
    // TODO: validate and propagate types.
    if (a.has_value() && b.has_value()) {
         return Expression(location, min_max(minimum, *a.value(), *b.value()));
    }
    else {
        return Expression(std::make_shared<MinMaxExpression>(location, a, b, minimum));
    }
}


std::optional<Expression> MinMaxExpression::evaluated(const EvaluationContext& context) const {
    auto new_a = a.evaluated(context);
    auto new_b = b.evaluated(context);
    if (new_a || new_b) {
        return create(location, new_a.value_or(a), new_b.value_or(b), minimum);
    }
    else {
        return {};
    }
}

void MinMaxExpression::serialize_sub(std::ostream& stream) const {
    stream << (minimum ? ".min" : ".max") << "(" << a << ", " << b << ")";
}

std::optional<Value> MinMaxExpression::minimum_value() const {
    const auto a_min = a.minimum_value();
    const auto b_min = b.minimum_value();

    if (a_min && b_min) {
        return min_max(minimum, *a_min, *b_min);
    }
    else {
        return {};
    }
}

std::optional<Value> MinMaxExpression::maximum_value() const {
    const auto a_max = a.maximum_value();
    const auto b_max = b.maximum_value();

    if (a_max && b_max) {
        return min_max(minimum, *a_max, *b_max);
    }
    else {
        return {};
    }
}

Value MinMaxExpression::min_max(bool minimum, const Value& a, const Value& b) {
    auto a_smaller = a < b;

    return (minimum && a_smaller) || (!minimum && !a_smaller) ? a : b;
}
