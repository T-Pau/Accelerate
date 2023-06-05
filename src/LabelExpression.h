//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_EXPRESSION_H
#define LABEL_EXPRESSION_H

#include "BaseExpression.h"
#include "LabelBody.h"

class LabelExpression: public BaseExpression {
public:
    static Expression create(Location location, const Object* object, std::shared_ptr<Label> label);
    static Expression create(Location location, const Object* object, Symbol label_name);
    LabelExpression(Location location, Symbol object_name, Symbol label_name): BaseExpression(location), type(LabelExpressionType::NAMED), unresolved_object_name(object_name), unresolved_label_name(label_name) {}
    LabelExpression(Location location, Symbol object_name, std::shared_ptr<Label> label): BaseExpression(location), type(LabelExpressionType::NAMED), unresolved_object_name(object_name), label(std::move(label)) {}
    LabelExpression(Location location, const Object* object, Symbol label_name): BaseExpression(location), type(LabelExpressionType::NAMED), object(object), unresolved_label_name(label_name) {}
    LabelExpression(Location location, const Object* object, std::shared_ptr<Label> label): BaseExpression(location), type(LabelExpressionType::NAMED), object(std::move(object)), label(std::move(label)) {}
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
    mutable LabelExpressionType type;
    Symbol unresolved_object_name;
    Symbol unresolved_label_name;
    const Object* object = nullptr;
    mutable std::shared_ptr<Label> label;
};

#endif // LABEL_EXPRESSION_H
