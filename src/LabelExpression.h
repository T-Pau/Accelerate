//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_EXPRESSION_H
#define LABEL_EXPRESSION_H

#include "Expression.h"
#include "LabelBodyElement.h"

class LabelExpression: public Expression {
public:
    explicit LabelExpression(std::shared_ptr<LabelBodyElement> label): label(std::move(label)) {}
    static std::shared_ptr<Expression> create(const std::shared_ptr<LabelBodyElement>& label);

    [[nodiscard]] Type type() const override {return LABEL;}
    [[nodiscard]] std::optional<Value> minimum_value() const override {return label->minimum_value();}
    [[nodiscard]] std::optional<Value> maximum_value() const override {return label->maximum_value();}
    [[nodiscard]] std::optional<Value> value() const override {return label->value();}

    void collect_variables(std::vector<Symbol>& variables) const override {}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override;
    void serialize_sub(std::ostream& stream) const override;


private:
    std::shared_ptr<LabelBodyElement> label;
};


#endif // LABEL_EXPRESSION_H
