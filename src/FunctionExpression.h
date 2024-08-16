/*
FunctionExpression.h --

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

#ifndef FUNCTION_EXPRESSION_H
#define FUNCTION_EXPRESSION_H

#include "Expression.h"

class FunctionExpression: public BaseExpression {
public:
    FunctionExpression(const Location& location, Symbol name, std::vector<Expression> arguments): BaseExpression(location), name(name), arguments(std::move(arguments)) {}

    void collect_objects(std::unordered_set<Object*>& objects) const override;

    static void setup(FileTokenizer& tokenizer);

protected:
    static Expression create(const Location& location, Symbol name, const std::vector<Expression>& arguments);
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;

    void serialize_sub(std::ostream& stream) const override;

    friend class Expression;

private:
    Symbol name;
    std::vector<Expression> arguments;

    static const std::unordered_map<Symbol, Expression (*)(const Location& location, const std::vector<Expression>&)> builtin_functions;
};


#endif // FUNCTION_EXPRESSION_H
