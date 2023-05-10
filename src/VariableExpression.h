/*
VariableExpression.h -- 

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

#ifndef VARIABLE_EXPRESSION_H
#define VARIABLE_EXPRESSION_H

#include "BaseExpression.h"
#include "Expression.h"

class VariableExpression: public BaseExpression {
public:
    explicit VariableExpression(Symbol symbol): symbol(symbol) {}

    [[nodiscard]] Type type() const override {return VARIABLE;}

    [[nodiscard]] Symbol variable() const {return symbol;}

    void collect_variables(std::vector<Symbol>& variables) const override {variables.emplace_back(symbol);}

protected:
    [[nodiscard]] std::optional<Expression> evaluated(const Environment &environment) const override;

    void serialize_sub(std::ostream& stream) const override {stream << symbol.str();}

    friend class Expression;

private:
    Symbol symbol;
};

#endif // VARIABLE_EXPRESSION_H
