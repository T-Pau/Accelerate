//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_EXPRESSION_H
#define LABEL_EXPRESSION_H

#include "BaseExpression.h"
#include "LabelBodyElement.h"

class LabelExpression: public BaseExpression {
public:
    explicit LabelExpression(std::shared_ptr<LabelBodyElement> label): label(std::move(label)) {}

    [[nodiscard]] Type type() const override {return LABEL;}
    [[nodiscard]] std::optional<Value> minimum_value() const override {return label->minimum_value();}
    [[nodiscard]] std::optional<Value> maximum_value() const override {return label->maximum_value();}
    [[nodiscard]] std::optional<Value> value() const override {return label->value();}

protected:
    static Expression create(const std::shared_ptr<LabelBodyElement>& label);
    [[nodiscard]] std::optional<Expression> evaluated(const Environment &environment) const override;
    void serialize_sub(std::ostream& stream) const override;


private:
    std::shared_ptr<LabelBodyElement> label;
};


#endif // LABEL_EXPRESSION_H
