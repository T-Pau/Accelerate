#ifdef IN_XLR8_LABEL_OFFSET_EXPRESSION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_LABEL_OFFSET_EXPRESSION_H
#ifndef HAD_XLR8_LABEL_OFFSET_EXPRESSION_H
#define HAD_XLR8_LABEL_OFFSET_EXPRESSION_H

/*
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

#include "BaseExpression.h"
#include "Body/Body.h"
#include "EvaluationContext.h"

/**
 * @brief Represents a label expression, which refers to a label in an object.
 *
 * This is the Expression entered into the environment for a label definition. It refers back to the defining BodyElement, which is used to compute the label's value.
 */
class LabelOffsetExpression: public BaseExpression {
public:
    static Expression create(const Location& location, Symbol entity_name, Symbol label_name, Body body) {return Expression(std::make_shared<LabelOffsetExpression>(location, entity_name, label_name, 0, std::move(body)));}

    static Expression create(const Location& location, Symbol entity_name, size_t unnamed_label_index, Body body) {return Expression(std::make_shared<LabelOffsetExpression>(location, entity_name, Symbol(), unnamed_label_index, std::move(body)));}

    LabelOffsetExpression(const Location& location, Symbol entity_name, Symbol label_name, size_t unnamed_label_index, Body body): BaseExpression(location), entity_name(entity_name), label_name(label_name), unnamed_label_index(unnamed_label_index), body(std::move(body)) {}


    [[nodiscard]] std::optional<Value> minimum_value() const override {return body.offset().minimum_value();}
    [[nodiscard]] std::optional<Value> maximum_value() const override {return body.offset().maximum_value();}
    [[nodiscard]] std::optional<Value> value() const override {return body.offset().value();}
    [[nodiscard]] std::optional<Value::Type> type() const override {return Value::UNSIGNED;}
    [[nodiscard]] bool has_value() const override {return body.offset().has_size();}

protected:
    [[nodiscard]] std::optional<Expression> evaluate(const EvaluationContext& context) override;
    void serialize_sub(std::ostream& stream) const override;

private:
    Symbol entity_name;
    Symbol label_name;
    size_t unnamed_label_index{0};
    Body body;
};

#endif // HAD_XLR8_LABEL_OFFSET_EXPRESSION_H
#undef IN_XLR8_LABEL_OFFSET_EXPRESSION_H
