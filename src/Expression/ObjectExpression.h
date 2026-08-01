#ifdef IN_XLR8_OBJECT_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_OBJECT_EXPRESSION_H
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

#include "Entity/Object.h"
#include "Expression/BaseExpression.h"

/**
 * @brief Represents an expression referring to an object.
 *
 * Since Objects are not deleted before the end of the program and an ObjectExpression can be contained in the Object it refers to, we use raw pointers to avoid circular ownership.
 */
class ObjectExpression : public BaseExpression {
  public:
    explicit ObjectExpression(const Location& location, Object* object) : BaseExpression(location), object_(object) {}

    [[nodiscard]] static Expression create(const Location& location, Object* object) { return *simplify(location, object, true); }

    [[nodiscard]] bool has_value() const override { return object()->has_address() && object()->address->has_address(); }

    [[nodiscard]] std::optional<Value> value() const override;

    [[nodiscard]] std::optional<Value::Type> type() const override { return Value::UNSIGNED; }

    void serialize_sub(std::ostream& stream) const override { stream << object()->name; }

    [[nodiscard]] std::optional<Expression> evaluate(const EvaluationContext& context) override { return simplify(location, object(), false); }

    [[nodiscard]] Expression clone(const CloneContext& context) const override { return Expression(std::make_shared<ObjectExpression>(location, object())); }

    Object* object() const { return object_; }

  protected:
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value> minimum_value() const override;

  private:
    static std::optional<Expression> simplify(const Location& location, Object* object, bool always_create);

    Object* object_;
};

#endif // HAD_XLR8_OBJECT_EXPRESSION_H
#undef IN_XLR8_OBJECT_EXPRESSION_H
