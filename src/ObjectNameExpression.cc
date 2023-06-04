//
// Created by Dieter Baron on 04.06.23.
//

#include "ObjectNameExpression.h"

#include "EvaluationContext.h"
#include "Expression.h"
#include "Object.h"

std::optional<Expression> ObjectNameExpression::evaluated(const EvaluationContext& context) const {
    if (context.object) {
        return Expression(context.object->name);
    }
    else {
        return {};
    }
}

void ObjectNameExpression::serialize_sub(std::ostream& stream) const {
    stream << ".object_name";
}

Expression ObjectNameExpression::create(const Object* object) {
    if (object) {
        return Expression(object->name);
    }
    else {
        return Expression(std::make_shared<ObjectNameExpression>());
    }
}
