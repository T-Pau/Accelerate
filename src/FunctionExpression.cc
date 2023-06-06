//
// Created by Dieter Baron on 07.05.23.
//

#include "FunctionExpression.h"

#include "ObjectFileParser.h"
#include "EvaluationContext.h"
#include "Expression.h"
#include "Function.h"
#include "InRangeExpression.h"
#include "LabelExpression.h"

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
    if (name == ObjectFileParser::token_in_range.as_symbol()) {
        return InRangeExpression::create(arguments);
    }
    else if (name == ObjectFileParser::token_label_offset.as_symbol()) {
        return LabelExpression::create(arguments);
    }
    // TODO: built in functions
    return Expression(std::make_shared<FunctionExpression>(name, arguments));
}

std::optional<Expression> FunctionExpression::evaluated(const EvaluationContext& context) const {
    std::vector<Expression> new_arguments;
    auto changed = false;
    for (auto& argument: arguments) {
        auto new_argument = argument.evaluated(context);
        if (new_argument) {
            new_arguments.emplace_back(*new_argument);
            changed = true;
        }
        else {
            new_arguments.emplace_back(argument);
        }
    }

    auto function = context.environment->get_function(name);
    if (!function) {
        if (changed) {
            return Expression(name, new_arguments);
        }
        else {
            return {};
        }
    }

    return function->call(new_arguments);
}
