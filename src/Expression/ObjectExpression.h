#ifndef HAD_XLR8_OBJECT_EXPRESSION_H
#define HAD_XLR8_OBJECT_EXPRESSION_H

/*
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

#include "Expression/EntityExpression.h"
#include "Entity/Object.h"

/**
 * @brief Represents an expression referring to an object.
 */
class ObjectExpression: public EntityExpression {
public:
    explicit ObjectExpression(const Location& location, const Object* object): EntityExpression(location, object) {}
    [[nodiscard]] static Expression create(const Location& location, const Object* object);

    [[nodiscard]] bool has_value() const override {return object()->has_address();}
    [[nodiscard]] std::optional<Value> value() const override;
    [[nodiscard]] std::optional<Value::Type> type() const override {return Value::UNSIGNED;}

protected:
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value> minimum_value() const override;

private:
    friend class Expression;

    const Object* object() const {return static_cast<const Object*>(entity);}
};

#endif // HAD_XLR8_OBJECT_EXPRESSION_H
