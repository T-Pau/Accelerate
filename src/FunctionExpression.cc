//
// Created by Dieter Baron on 07.05.23.
//

#include "FunctionExpression.h"

#include "Expression.h"

void FunctionExpression::serialize_sub(std::ostream &stream) const {
    stream << name << "(";
    auto first = true;
    for (auto& argument: arguments) {
        if (!first) {
            stream << ", ";
        }
        stream << argument;
        first = false;
    }
    stream << ")";
}

void FunctionExpression::collect_objects(std::unordered_set<Object*> &objects) const {
    for (auto& argument: arguments) {
        argument.collect_objects(objects);
    }
}

Expression FunctionExpression::create(Symbol name, const std::vector<Expression> &arguments) {
    // TODO: built in functions
    return Expression(std::make_shared<FunctionExpression>(name, arguments));
}

std::optional<Expression> FunctionExpression::evaluated(const EvaluationContext& context) const {
    // TODO: implement
    return {};
}
