/*
SizeofExpression.cc -- 

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

#include "SizeofExpression.h"

#include "EvaluationContext.h"
#include "Expression.h"
#include "ObjectExpression.h"
#include "ParseException.h"
#include "VariableExpression.h"

Expression SizeofExpression::create(const std::vector<Expression>& arguments) {
    if (arguments.size() != 1) {
        throw ParseException(Location(), "invalid number of arguments");
    }
    auto& argument = arguments[0];

    if (!argument.is_variable()) {
        throw ParseException(argument.location(), "symbol argument required");
    }

    return Expression(std::make_shared<SizeofExpression>(argument.as_variable()->variable()));
}

Expression SizeofExpression::create(const Object* object) {
    const auto size_range = object->size_range();
    if (size_range.size()) {
        return Expression(Value(*size_range.size()));
    }
    else {
        return Expression(std::make_shared<SizeofExpression>(object));
    }
}

std::optional<Expression> SizeofExpression::evaluated(const EvaluationContext& context) const {
    if (!object) {
        if (const auto new_object = context.environment->get_variable(object_name)) {
            if (!new_object->is_object()) {
                throw Exception(".sizeof requires object");
            }
            return create(new_object->as_object()->object);
        }
    }
    return {};
}

std::optional<Value> SizeofExpression::maximum_value() const {
    if (object && object->size_range().maximum_value()) {
        return Value(*object->size_range().maximum_value());
    }
    else {
        return {};
    }
}

void SizeofExpression::serialize_sub(std::ostream& stream) const {
    stream << ".sizeof(" << object_name << ")";
}

std::optional<Value> SizeofExpression::minimum_value() const {
    if (object) {
        return Value(object->size_range().minimum_value());
    }
    else {
        return Value(uint64_t{0});
    }
}
