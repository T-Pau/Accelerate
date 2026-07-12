#ifdef IN_XLR8_SIZEOF_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_SIZEOF_EXPRESSION_H
#ifndef HAD_XLR8_SIZEOF_EXPRESSION_H
#define HAD_XLR8_SIZEOF_EXPRESSION_H

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

#include "BaseExpression.h"
#include "Entity/Object.h"

class Object;

/**
 * @brief Represents an expression that evaluates to the size of an object: `.sizeof(object)`.
 */
class SizeofExpression: public BaseExpression {
public:
    explicit SizeofExpression(const Location& location, Symbol object_name): BaseExpression(location), object_name{object_name} {}
    explicit SizeofExpression(const Location& location, const Object* object): BaseExpression(location), object_name{object->name}, object{object} {}

    static Expression create(const Location& location, const std::vector<Expression>& arguments);
    static Expression create(const Location& location, const Object* object);

    [[nodiscard]] std::optional<Expression> evaluate(const EvaluationContext& context) override;
    [[nodiscard]] std::optional<Value> minimum_value() const override {return object ? object->size_range().minimum_value() : std::optional<Value>{};}
    [[nodiscard]] bool has_value() const override {return object && object->size_range().size();}
    [[nodiscard]] std::optional<Value> value() const override {return object ? object->size_range().value() : std::optional<Value>{};}
    [[nodiscard]] std::optional<Value> maximum_value() const override {return object ? object->size_range().maximum_value() : std::optional<Value>{};}
    [[nodiscard]] std::optional<Value::Type> type() const override {return  Value::UNSIGNED;}

    void resolve(Scope* scope, Entity* containing_entity) override;

protected:
    void serialize_sub(std::ostream& stream) const override;

private:

    Symbol object_name;
    const Object* object{};
};

#endif // HAD_XLR8_SIZEOF_EXPRESSION_H
#undef IN_XLR8_SIZEOF_EXPRESSION_H
