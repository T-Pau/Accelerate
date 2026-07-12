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

#include "Entity/Callable.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "EvaluationContext.h"
#include "SequenceTokenizer.h"

using namespace tpau::cpp_kernal;

#define ARGUMENTS "arguments"

bool Callable::initialized = false;
Token Callable::token_arguments;

void Callable::initialize() {
    if (!initialized) {
        initialized = true;
        token_arguments = Token(Token::NAME, ARGUMENTS);
    }
}

Callable::Callable(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition): Entity(location, name, parent_scope, definition) {
    initialize();
    auto parameters = definition->as_dictionary();

    if (auto arguments_value = parameters->get_optional(token_arguments)) {
        if (!arguments_value->is_scalar()) {
            throw LocationException(arguments_value->location, "invalid arguments");
        }
        auto tokenizer = SequenceTokenizer(arguments_value->as_scalar()->tokens);
        arguments = Arguments(tokenizer);
        if (!tokenizer.ended()) {
            throw LocationException(tokenizer.next().location, "invalid arguments");
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
        if (auto value = default_argument(index)) {
            stream << " = " << *value;
        }
    }
}

EvaluationContext Callable::evaluation_context(EvaluationResult& result) {
    return Entity::evaluation_context(result).skipping_variables(arguments.names);
}

Callable::Arguments::Arguments(Tokenizer& tokenizer) {
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

std::ostream& operator<<(std::ostream& stream, const Callable::Arguments& arguments) {
    arguments.serialize(stream);
    return stream;
}
