//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_EXPRESSION_H
#define LABEL_EXPRESSION_H

#include "BaseExpression.h"
#include "LabelBody.h"

class LabelExpression: public BaseExpression {
public:
    explicit LabelExpression(std::shared_ptr<Label> label): label(std::move(label)) {}

    [[nodiscard]] std::optional<Value> minimum_value() const override {return Value(label->offset.minimum);}
    [[nodiscard]] std::optional<Value> maximum_value() const override;
    [[nodiscard]] std::optional<Value> value() const override;

protected:
    static Expression create(const std::shared_ptr<Label>& label);
    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const override;
    void serialize_sub(std::ostream& stream) const override;


private:
    std::shared_ptr<Label> label;
};


#endif // LABEL_EXPRESSION_H
