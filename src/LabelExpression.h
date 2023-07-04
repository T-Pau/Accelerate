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
    static Expression create(Location location, const Entity* object, std::shared_ptr<Label> label);
    static Expression create(Location location, const Entity* object, Symbol label_name);
    static Expression create(const std::vector<Expression>& arguments);
    LabelExpression(Location location, Symbol object_name, Symbol label_name): BaseExpression(location), type(LabelExpressionType::NAMED), unresolved_object_name(object_name), unresolved_label_name(label_name) {}
    LabelExpression(Location location, Symbol object_name, std::shared_ptr<Label> label): BaseExpression(location), type(LabelExpressionType::NAMED), unresolved_object_name(object_name), label(std::move(label)) {}
    LabelExpression(Location location, const Entity* object, Symbol label_name): BaseExpression(location), type(LabelExpressionType::NAMED), object(object), unresolved_label_name(label_name) {}
    LabelExpression(Location location, const Entity* object, std::shared_ptr<Label> label): BaseExpression(location), type(LabelExpressionType::NAMED), object(std::move(object)), label(std::move(label)) {}
    LabelExpression(Location location, LabelExpressionType type): BaseExpression(location), type(type) {}

    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value> value() const override;

    void set_label(std::shared_ptr<Label> new_label) const {label = std::move(new_label);}

protected:
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;
    void serialize_sub(std::ostream& stream) const override;

private:
    Symbol object_name() const;
    Symbol label_name() const {return label ? label->name : unresolved_label_name;}
    LabelExpressionType type;
    Symbol unresolved_object_name;
    Symbol unresolved_label_name;
    const Entity* object = nullptr;
    mutable std::shared_ptr<Label> label;
};

#endif // LABEL_EXPRESSION_H
