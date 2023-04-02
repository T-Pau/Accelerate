/*
IntegerExpression.h -- 

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

#ifndef INTEGER_EXPRESSION_H
#define INTEGER_EXPRESSION_H

#include "Expression.h"

class IntegerExpression: public Expression {
public:
    explicit IntegerExpression(const Token& token);
    explicit IntegerExpression(int64_t value): value_(value) {set_byte_size(Int::minimum_byte_size(value));}

    [[nodiscard]] Type type() const override {return INTEGER;}

    [[nodiscard]] bool has_value() const override {return true;}
    [[nodiscard]] int64_t value() const override {return value_;}
    [[nodiscard]] size_t minimum_byte_size() const override {return Int::minimum_byte_size(value());}
    void replace_variables(Symbol (*transform)(Symbol)) override {}

    void collect_variables(std::vector<Symbol>& variables) const override {}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override {return {};}
    [[nodiscard]] std::shared_ptr<Expression> clone() const override;

    void serialize_sub(std::ostream& stream) const override;

private:
    int64_t value_;
};

#endif // INTEGER_EXPRESSION_H
