//
// Created by Dieter Baron on 29.05.23.
//

#include "Callable.h"

#include "EvaluationContext.h"
#include "Exception.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

#define ARGUMENTS "arguments"

bool Callable::initialized = false;
Token Callable::token_arguments;

void Callable::initialize() {
    if (!initialized) {
        initialized = true;
        token_arguments = Token(Token::NAME, ARGUMENTS);
    }
}

Callable::Callable(Token name_, const std::shared_ptr<ParsedValue>& definition): Entity(name_, definition) {
    initialize();
    auto parameters = definition->as_dictionary();

    auto arguments_value = parameters->get_optional(token_arguments);
    if (arguments_value) {
        if (!arguments_value->is_scalar()) {
            throw ParseException(arguments_value->location, "invalid arguments");
        }
        auto tokenizer = SequenceTokenizer(arguments_value->as_scalar()->tokens);
        arguments = Arguments(tokenizer);
        if (!tokenizer.ended()) {
            throw ParseException(tokenizer.next(), "invalid arguments");
        }
    }
}


std::optional<Expression> Callable::Arguments::default_argument(size_t index) const {
    if (index >= minimum_arguments()) {
        return default_arguments[index - minimum_arguments()];
    }
    else {
        return {};
    }
}


void Callable::serialize_callable(std::ostream& stream) const {
    serialize_entity(stream);
    if (!arguments.empty()) {
        stream << "    " ARGUMENTS ": " << arguments << std::endl;
    }
}

void Callable::Arguments::serialize(std::ostream& stream) const {
    for (size_t index = 0; index < names.size(); index++) {
        if (index > 0) {
            stream << ", ";
        }
        stream << name(index);
        auto value = default_argument(index);
        if (value) {
            stream << " = " << *value;
        }
    }
}

EvaluationContext Callable::bind(const std::vector<Expression>& actual_arguments) const {
    if (actual_arguments.size() < arguments.minimum_arguments() || actual_arguments.size() > arguments.maximum_arguments()) {
        throw Exception("invalid number of actual_arguments");
    }
    auto environment = std::make_shared<Environment>();
    for (size_t index = 0; index < arguments.maximum_arguments(); index++) {
        if (index >= actual_arguments.size()) {
            environment->add(argument_name(index), *default_argument(index));
        }
        else {
            environment->add(argument_name(index), actual_arguments[index]);
        }
    }

    return EvaluationContext(environment, true);
}

Callable::Arguments::Arguments(Tokenizer& tokenizer) {
    auto had_default_argument = false;

    while (!tokenizer.ended()) {
        auto argument_name = tokenizer.expect(Token::NAME);
        auto default_argument = std::optional<Expression>();
        auto token = tokenizer.next();
        if (token == Token::equals) {
            default_argument = Expression(tokenizer);
            token = tokenizer.next();
        }

        if (had_default_argument && !default_argument) {
            throw ParseException(argument_name, "required argument cannot follow optional argument");
        }

        names.emplace_back(argument_name.as_symbol());
        if (default_argument) {
            had_default_argument = true;
            default_arguments.emplace_back(*default_argument);
        }

        if (token != Token::comma) {
            tokenizer.unget(token);
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const Callable::Arguments& arguments) {
    arguments.serialize(stream);
    return stream;
}
