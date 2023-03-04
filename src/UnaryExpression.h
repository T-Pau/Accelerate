/*
UnaryExpression.h -- 

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

#ifndef UNARY_EXPRESSION_H
#define UNARY_EXPRESSION_H

#include "Expression.h"

class UnaryExpression: public Expression {
public:
    enum Operation {
        BANK_BYTE,
        BITWISE_NOT,
        HIGH_BYTE,
        LOW_BYTE,
        MINUS,
        PLUS
    };

    UnaryExpression(Operation operation, std::shared_ptr<Expression>operand);
    [[nodiscard]] std::shared_ptr<Expression> static create(Operation operation, std::shared_ptr<Expression> operand, size_t byte_size);

    [[nodiscard]] Type type() const override {return UNARY;}
    [[nodiscard]] size_t minimum_byte_size() const override;
    void replace_variables(symbol_t (*transform)(symbol_t)) override {operand->replace_variables(transform);}

    void collect_variables(std::vector<symbol_t>& variables) const override { operand->collect_variables(variables);}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override;
    [[nodiscard]] std::shared_ptr<Expression> clone() const override {return create(operation, operand, byte_size());}

    void serialize_sub(std::ostream& stream) const override;

private:
    Operation operation;
    std::shared_ptr<Expression> operand;
};


#endif // UNARY_EXPRESSION_H
