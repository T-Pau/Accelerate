/*
LabelExpression.h --

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

#ifndef LABEL_EXPRESSION_H
#define LABEL_EXPRESSION_H

#include "BaseExpression.h"
#include "LabelBody.h"

class LabelExpression: public BaseExpression {
public:
    static Expression create(Location location, const Entity* object, Symbol label_name, SizeRange offset, SizeRange scope_offset = SizeRange(0), bool keep = false);
    static Expression create(const std::vector<Expression>& arguments);
    static Expression create(Location location, LabelExpressionType type, size_t unnamed_index = std::numeric_limits<size_t>::max(), SizeRange offset = SizeRange(0, {}));
    LabelExpression(Location location, Symbol object_name, Symbol label_name): BaseExpression(location), label_type(LabelExpressionType::NAMED), object_name(object_name), label_name(label_name) {}
    LabelExpression(Location location, Symbol object_name, Symbol label_name, SizeRange offset): BaseExpression(location), label_type(LabelExpressionType::NAMED), object_name(object_name), label_name(label_name), offset(offset) {}
    LabelExpression(Location location, const Entity* object, Symbol label_name, SizeRange offset);
    LabelExpression(Location location, LabelExpressionType type, size_t unnamed_index, SizeRange offset): BaseExpression(location), label_type(type), offset(offset), unnamed_index(unnamed_index) {}

    [[nodiscard]] std::optional<Value> minimum_value() const override {return offset.minimum_value();}
    [[nodiscard]] std::optional<Value> maximum_value() const override {return offset.maximum_value();}
    [[nodiscard]] std::optional<Value> value() const override {return offset.value();}
    [[nodiscard]] std::optional<Value::Type> type() const override {return Value::UNSIGNED;}

protected:
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;
    void serialize_sub(std::ostream& stream) const override;

private:
    [[nodiscard]] bool added_to_environment() const {return unnamed_index != std::numeric_limits<size_t>::max();}

    LabelExpressionType label_type;
    Symbol object_name;
    Symbol label_name;
    const Entity* object = nullptr;
    SizeRange offset = SizeRange(0, {});
    size_t unnamed_index = std::numeric_limits<size_t>::max();
};

#endif // LABEL_EXPRESSION_H
