/*
VariableExpression.cc -- 

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

#include "VariableExpression.h"

#include "EvaluationContext.h"
#include "Object.h"
#include "ParseException.h"

std::optional<Expression> VariableExpression::evaluated(const EvaluationContext& context) const {
    if (symbol == Token::colon_minus.as_symbol() || symbol == Token::colon_plus.as_symbol()) {
        if (!context.entity) {
            return {};
        }
        return Expression(Expression(context.entity->name), Expression::BinaryOperation::ADD, Expression(location, symbol == Token::colon_minus.as_symbol() ? LabelExpressionType::PREVIOUS_UNNAMED : LabelExpressionType::NEXT_UNNAMED));
    }
    if (context.evaluating(symbol)) {
        throw Exception("circular definition of %s", symbol.c_str());
    }

    auto value = (*context.environment)[symbol];

    if (value) {
        auto new_value = *value;
        if (!context.shallow()) {
            new_value.evaluate(context.evaluating_variable(symbol));
        }
        return new_value;
    }
    else {
        context.result.unresolved_variables.insert(symbol);
        return {};
    }
}
