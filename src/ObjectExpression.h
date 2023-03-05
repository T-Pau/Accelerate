/*
ObjectExpression.h --

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

#ifndef LINKER_OBJECT_EXPRESSION_H
#define LINKER_OBJECT_EXPRESSION_H

#include "Expression.h"
#include "Object.h"

class ObjectExpression: public Expression {
public:
    explicit ObjectExpression(const Object* object): object(object) {}

    [[nodiscard]] Type type() const override {return OBJECT;}

    [[nodiscard]] bool has_value() const override {return object->has_address();}
    [[nodiscard]] int64_t value() const override {return has_value() ? static_cast<int64_t>(object->address.value()) : 0;}
    [[nodiscard]] size_t minimum_byte_size() const override;
    void replace_variables(symbol_t (*transform)(symbol_t)) override {}
    void collect_variables(std::vector<symbol_t>& variables) const override {}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override {return {};}
    [[nodiscard]] std::shared_ptr<Expression> clone() const override;

    void serialize_sub(std::ostream& stream) const override {stream << object->name.as_symbol();}

private:
    const Object* object;
};

#endif // LINKER_OBJECT_EXPRESSION_H
