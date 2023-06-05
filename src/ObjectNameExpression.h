//
// Created by Dieter Baron on 04.06.23.
//

#ifndef OBJECT_NAME_EXPRESSION_H
#define OBJECT_NAME_EXPRESSION_H

#include "BaseExpression.h"

class ObjectNameExpression: public BaseExpression {
  public:
    ObjectNameExpression() = default;
    ObjectNameExpression(Location location): BaseExpression(location) {}

    static Expression create(Object* object);

    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext &context) const override;
    void serialize_sub(std::ostream &stream) const override;
};


#endif // OBJECT_NAME_EXPRESSION_H
