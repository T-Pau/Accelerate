//
// Created by Dieter Baron on 29.05.23.
//

#include "Callable.h"

#include "EvaluationContext.h"
#include "Exception.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

bool Callable::initialized = false;
Token Callable::token_arguments;
Token Callable::token_visibility;

void Callable::initialize() {
    if (!initialized) {
        initialized = true;
        token_arguments = Token(Token::NAME, "arguments");
        token_visibility = Token(Token::NAME, "visibility");
        VisibilityHelper::initialize();
    }
}

Callable::Callable(Token name_, const std::shared_ptr<ParsedValue>& definition) {
    initialize();
    auto parameters = definition->as_dictionary();

    name = name_;

    auto visibility_value = (*parameters)[token_visibility]->as_singular_scalar()->token();
    auto visibility_ = VisibilityHelper::from_token(visibility_value);
    if (!visibility_) {
        throw ParseException(visibility_value, "invalid visibility");
    }
    visibility = *visibility_;

    auto arguments_value = parameters->get_optional(token_arguments);
    if (arguments_value) {
        if (!arguments_value->is_scalar()) {
            throw ParseException(arguments_value->location, "invalid arguments");
        }
        auto tokenizer = SequenceTokenizer(arguments_value->as_scalar()->tokens);
        parse_arguments(tokenizer, true);
        if (!tokenizer.ended()) {
            throw ParseException(tokenizer.next(), "invalid arguments");
        }
    }
}

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

void Callable::parse_arguments(Tokenizer& tokenizer, bool comma_separated) {
    auto had_default_argument = false;

    while (!tokenizer.ended()) {
        auto argument_name = tokenizer.expect(Token::NAME, TokenGroup::newline);
        auto default_argument = std::optional<Expression>();
        auto token = tokenizer.next();
        if (token == Token::equals) {
            default_argument = Expression(tokenizer);
            token = tokenizer.next();
        }

        if (had_default_argument && !default_argument) {
            throw ParseException(argument_name, "required argument cannot follow optional argument");
        }

        argument_names.emplace_back(argument_name.as_symbol());
        if (default_argument) {
            had_default_argument = true;
            default_arguments.emplace_back(*default_argument);
        }

        if (comma_separated) {
            if (token != Token::comma) {
                break;
            }
        }
        else {
            tokenizer.unget(token);
            if (!token.is_name()) {
                break;
            }
        }
    }
}
