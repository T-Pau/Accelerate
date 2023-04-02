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

#include "ParseException.h"

std::shared_ptr<Expression> VariableExpression::clone() const {
    auto node = std::make_shared<VariableExpression>(symbol);
    node->set_byte_size(byte_size());
    return node;
}



VariableExpression::VariableExpression(const Token &token) {
    if (!token.is_name()) {
        throw ParseException(token, "internal error: creating VariableExpression from non-name token");
    }
    location = token.location;
    symbol = token.as_symbol();
}

std::shared_ptr<Expression> VariableExpression::evaluate(const Environment &environment) const {
    auto value = environment[symbol];

    if (value) {
        // TODO: Detect loops
        auto node = Expression::evaluate(value, environment);

        if (byte_size() != 0 && node->byte_size() != byte_size()) {
            node = node->clone();
            node->set_byte_size(byte_size());
        }
        return node;
    }
    else {
        return {};
    }
}

void VariableExpression::replace_variables(Symbol (*transform)(Symbol)) {
    symbol = transform(symbol);
}

