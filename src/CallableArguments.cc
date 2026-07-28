/*
Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CallableArguments.h"

#include "Entity/Constant.h"
#include "Expression/ArgumentExpression.h"
#include "Expression/ConstantExpression.h"
#include "Scope.h"
#include "SequenceTokenizer.h"
#include "StructuredDictionary.h"
#include "StructuredScalar.h"

Token CallableArguments::token_arguments = {Token::NAME, "arguments"};

CallableArguments::CallableArguments(Tokenizer& tokenizer) {
    auto had_default_argument = false;

    while (!tokenizer.ended()) {
        auto argument_name = tokenizer.next();
        if (!argument_name.is_name()) {
            tokenizer.unget(argument_name);
            break;
        }
        auto default_argument = std::optional<Expression>();
        auto token = tokenizer.next();
        if (token == Token::equals) {
            default_argument = Expression(tokenizer);
            token = tokenizer.next();
        }

        if (had_default_argument && !default_argument) {
            throw LocationException(argument_name.location, "required argument cannot follow optional argument");
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

CallableArguments::CallableArguments(ScopeEntity* entity, const std::shared_ptr<StructuredValue>& definition) : entity(entity) {
    auto parameters = definition->as_dictionary();

    if (auto arguments_value = parameters->get_optional(token_arguments)) {
        if (!arguments_value->is_scalar()) {
            throw LocationException(arguments_value->location, "invalid arguments");
        }
        auto tokenizer = SequenceTokenizer(arguments_value->as_scalar()->tokens);
        *this = CallableArguments(tokenizer);
        if (!tokenizer.ended()) {
            throw LocationException(tokenizer.next().location, "invalid arguments");
        }
    }
}

void CallableArguments::serialize_callable(std::ostream& stream) const {
    if (!empty()) {
        stream << "    " << CallableArguments::token_arguments.as_symbol() << ": " << *this << std::endl;
    }
}

void CallableArguments::serialize(std::ostream& stream) const {
    for (size_t index = 0; index < names.size(); index++) {
        if (index > 0) {
            stream << ", ";
        }
        stream << name(index);
        if (auto value = default_argument(index)) {
            stream << " = " << *value;
        }
    }
}

std::optional<Expression> CallableArguments::default_argument(size_t index) const {
    if (index < minimum_arguments()) {
        return {};
    }
    return default_arguments[index - minimum_arguments()];
}

std::ostream& operator<<(std::ostream& stream, const CallableArguments& arguments) {
    arguments.serialize(stream);
    return stream;
}

void CallableArguments::enter_arguments() {
    for (const auto& argument_name : argument_names()) {
        entity->add(std::make_shared<Constant>(entity->location, argument_name, Visibility::ENTITY, entity->scope(), false, ArgumentExpression::create(entity->location, argument_name)));
    }
}

void CallableArguments::clear_arguments() {
    for (const auto& argument_name : argument_names()) {
        auto argument = get(entity, argument_name);
        argument->clear_argument();
    }
}

std::vector<std::shared_ptr<Constant>> CallableArguments::create_argument_constants(std::shared_ptr<Scope> scope, const std::vector<Expression>& arguments) const {
    if (arguments.size() < minimum_arguments() || arguments.size() > maximum_arguments()) {
        throw LocationException(entity->location, "invalid number of arguments for {}: expected {} to {}, got {}", entity->name, minimum_arguments(), maximum_arguments(), arguments.size());
    }

    auto constants = std::vector<std::shared_ptr<Constant>>();

    for (size_t index = 0; index < arguments.size(); index++) {
        auto argument_name = name(index);
        auto constant = std::make_shared<Constant>(entity->location, argument_name, Visibility::ARGUMENT, scope, false, arguments[index]);
        constants.emplace_back(constant);
    }
    for (size_t index = arguments.size(); index < maximum_arguments(); index++) {
        auto argument_name = name(index);

        // TODO: use pre-created constants for default arguments
        auto constant = std::make_shared<Constant>(entity->location, argument_name, Visibility::ARGUMENT, scope, false, default_argument(index).value());
        constants.emplace_back(constant);
    }
    return constants;
}

void CallableArguments::set_argument_expressions(const std::vector<std::shared_ptr<Constant>>& arguments) {
    for (size_t index = 0; index < arguments.size(); index++) {
        auto argument_name = name(index);
        auto argument = get(entity, argument_name);
        auto constant = arguments[index];
        argument->set_argument(ConstantExpression::create(argument->location, constant));
    }
}

ArgumentExpression* CallableArguments::get(ScopeEntity* entity, Symbol name) const {
    auto argument = entity->scope()->get_constant(name);
    if (!argument) {
        throw LocationException(entity->location, "internal error: argument {} not found in {}", name, entity->name);
    }
    auto argument_expression = argument->value.as<ArgumentExpression>();
    if (!argument_expression) {
        throw LocationException(entity->location, "internal error: argument {} is not an ArgumentExpression in {}", name, entity->name);
    }
    return argument_expression;
}

void CallableArguments::traverse(std::function<void(Expression&)> expression_callback) {
    for (auto& default_argument : default_arguments) {
        expression_callback(default_argument);
    }
}
