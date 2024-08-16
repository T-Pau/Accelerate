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

Expression SizeofExpression::create(const Location& location, const std::vector<Expression>& arguments) {
    if (arguments.size() != 1) {
        throw ParseException(location, "invalid number of arguments");
    }
    auto& argument = arguments[0];

    if (!argument.is_variable()) {
        throw ParseException(argument.location(), "symbol argument required");
    }

    return Expression(std::make_shared<SizeofExpression>(location, argument.as_variable()->variable()));
}


Expression SizeofExpression::create(const Location& location, const Object* object) {
    const auto size_range = object->size_range();
    if (size_range.size()) {
        return Expression(location, Value(*size_range.size()));
    }
    else {
        return Expression(std::make_shared<SizeofExpression>(location, object));
    }
}


std::optional<Expression> SizeofExpression::evaluated(const EvaluationContext& context) const {
    if (object) {
        if (size_range != object->size_range()) {
            return create(location, object);
        }
        else {
            return {};
        }
    }
    else if (const auto new_object = context.environment->get_variable(object_name)) {
        if (!new_object->is_object()) {
            throw ParseException(location, ".sizeof requires object");
        }
        return create(location, new_object->as_object()->object);
    }
    else {
        return {};
    }
}


void SizeofExpression::serialize_sub(std::ostream& stream) const { stream << ".sizeof(" << object_name << ")"; }
