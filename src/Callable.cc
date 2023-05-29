//
// Created by Dieter Baron on 29.05.23.
//

#include "Callable.h"

#include "Exception.h"
#include "EvaluationContext.h"

std::optional<Expression> Callable::default_argument(size_t index) const {
    if (index >= minimum_arguments()) {
        return default_arguments[index - minimum_arguments()];
    }
    else {
        return {};
    }
}

void Callable::serialize_callable(std::ostream& stream) const {
    stream << "    visibility: " << visibility << std::endl;
    if (!argument_names.empty()) {
        stream << "    arguments: ";
        for (size_t index = 0; index < argument_names.size(); index++) {
            if (index > 0) {
                stream << ", ";
            }
            stream << argument_name(index);
            auto value = default_argument(index);
            if (value) {
                stream << " = " << *value;
            }
        }
        stream << std::endl;
    }
}

EvaluationContext Callable::bind(const std::vector<Expression>& arguments) const {
    if (arguments.size() < minimum_arguments() || arguments.size() > maximum_arguments()) {
        throw Exception("invalid number of arguments");
    }
    auto environment = std::make_shared<Environment>();
    for (size_t index = 0; index < maximum_arguments(); index++) {
        if (index >= arguments.size()) {
            environment->add(argument_name(index), *default_argument(index));
        }
        else {
            environment->add(argument_name(index), arguments[index]);
        }
    }

    return EvaluationContext(environment, true);
}
