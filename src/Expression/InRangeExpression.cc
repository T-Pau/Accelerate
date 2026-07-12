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

#include "InRangeExpression.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "ValueExpression.h"

using namespace tpau::cpp_kernal;

Expression InRangeExpression::create(const Location& location, const std::vector<Expression>& arguments) {
    if (arguments.size() != 3) {
        throw LocationException(location, "invalid number of arguments for .in_range()");
    }
    return create(location, arguments[0], arguments[1], arguments[2]);
}


std::optional<Expression> InRangeExpression::simplify(const Location& location, const Expression& lower_bound, const Expression& upper_bound, const Expression& argument, bool always_create) {
    if (!lower_bound.has_type(Value::NUMBER).value_or(true) ||
        !upper_bound.has_type(Value::NUMBER).value_or(true) ||
        !argument.has_type(Value::NUMBER).value_or(true)) {
        throw LocationException(location, "arguments to .in_range() must be numeric");
    }

    if (lower_bound.maximum_value() <= argument.minimum_value() && upper_bound.minimum_value() >= argument.maximum_value()) {
        return ValueExpression::create(location, Value(true));
    }
    else if (lower_bound.minimum_value() > argument.maximum_value() || upper_bound.maximum_value() < argument.minimum_value()) {
        return ValueExpression::create(location, Value(false));
    }
    else if (always_create) {
        return Expression(std::make_shared<InRangeExpression>(location, lower_bound, upper_bound, argument));
    }
    else {
        return {};
    }
}

std::optional<Expression> InRangeExpression::evaluate_process(const EvaluationContext& context) {
    return simplify(location, lower_bound, upper_bound, argument, false);
}

void InRangeExpression::serialize_sub(std::ostream &stream) const {
    stream << ".in_range(" << lower_bound << ", " << upper_bound << ", " << argument << ")";
}
