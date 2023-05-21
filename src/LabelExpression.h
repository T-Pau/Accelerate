//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_EXPRESSION_H
#define LABEL_EXPRESSION_H

#include "BaseExpression.h"
#include "LabelBody.h"

class LabelExpression: public BaseExpression {
public:

    explicit LabelExpression(Location location, std::shared_ptr<Label> label): BaseExpression(location), type(LabelExpressionType::NAMED), label(std::move(label)) {}
    explicit LabelExpression(Location location, LabelExpressionType type): BaseExpression(location), type(type) {}

    [[nodiscard]] std::optional<Value> minimum_value() const override;
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value> value() const override;

    void set_label(std::shared_ptr<Label> new_label) const {label = std::move(new_label);}

protected:
    static Expression create(const std::shared_ptr<Label>& label);
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;
    void serialize_sub(std::ostream& stream) const override;

private:
    mutable LabelExpressionType type;
    mutable std::shared_ptr<Label> label;
};

#endif // LABEL_EXPRESSION_H
