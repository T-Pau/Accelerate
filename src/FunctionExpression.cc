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

void FunctionExpression::collect_variables(std::vector<Symbol> &variables) const {
    for (auto& argument: arguments) {
        argument.collect_variables(variables);
    }
}

Expression FunctionExpression::create(Symbol name, const std::vector<Expression> &arguments) {
    // TODO: built in functions
    return Expression(std::make_shared<FunctionExpression>(name, arguments));
}

std::optional<Expression> FunctionExpression::evaluated(const Environment &environment) const {
    // TODO: implement
    return {};
}
