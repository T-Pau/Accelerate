/*
InRangeExpression.cc --

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
#include "ValueExpression.h"

std::shared_ptr<Expression> InRangeExpression::create(const std::shared_ptr<Expression> &lower_bound,
                                                      const std::shared_ptr<Expression> &upper_bound,
                                                      const std::shared_ptr<Expression> &argument) {
    if (lower_bound->maximum_value() <= argument->minimum_value() && upper_bound->minimum_value() >= argument->maximum_value()) {
        return std::make_shared<ValueExpression>(Value(true));
    }
    else if (lower_bound->minimum_value() > argument->maximum_value() || upper_bound->maximum_value() < argument->minimum_value()) {
        return std::make_shared<ValueExpression>(Value(false));
    }
    else {
        return std::make_shared<InRangeExpression>(lower_bound, upper_bound, argument);
    }
}

std::shared_ptr<Expression> InRangeExpression::evaluate(const Environment &environment) const {
    auto new_lower = lower_bound->evaluate(environment);
    auto new_upper = upper_bound->evaluate(environment);
    auto new_argument = argument->evaluate(environment);

    if (new_lower || new_upper || new_argument) {
        return create(new_lower ? new_lower : lower_bound, new_upper ? new_upper : upper_bound, new_argument ? new_argument : argument);
    }
    else {
        return {};
    }
}

void InRangeExpression::serialize_sub(std::ostream &stream) const {
    stream << ".in_range(" << lower_bound << ", " << upper_bound << ", " << argument << ")";
}

void InRangeExpression::collect_variables(std::vector<Symbol> &variables) const {
    lower_bound->collect_variables(variables);
    upper_bound->collect_variables(variables);
    argument->collect_variables(variables);
}
