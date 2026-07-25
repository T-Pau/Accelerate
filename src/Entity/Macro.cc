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

#include "Entity/Macro.h"

#include "StructuredBody.h"
#include "StructuredDictionary.h"

bool Macro::initialized = false;
Token Macro::token_body;

void Macro::initialize() {
    if (!initialized) {
        initialized = true;
        token_body = Token(Token::NAME, "body");
    }
}

Macro::Macro(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> containing_scope, bool default_only, CallableArguments arguments) : ScopeEntity(location, name, visibility, containing_scope, default_only), arguments(std::move(arguments)) { this->arguments.enter_arguments(this); }

Macro::Macro(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition) : ScopeEntity(location, name, parent_scope, definition) {
    initialize();

    arguments = CallableArguments(definition);

    auto parameters = definition->as_dictionary();

    body = (*parameters)[token_body]->as_body()->body;

    arguments.enter_arguments(this);
}

std::ostream& operator<<(std::ostream& stream, const Macro& macro) {
    macro.serialize(stream);
    return stream;
}

Body Macro::expand(const std::vector<Expression>& arguments, std::shared_ptr<Scope> outer_environment) const {
#if 0
    EvaluationResult result;
    auto inner_environment = std::make_shared<Scope>(*environment);
    inner_environment->clear_next();
    inner_environment->add_next(std::move(outer_environment));
    return body.evaluate(EvaluationContext(result, EvaluationContext::MACRO_EXPANSION, bind(arguments))).value_or(body).scoped(inner_environment);
#else
    return {};
#endif
}

void Macro::serialize(std::ostream& stream) const {
    stream << ".macro " << name << " {" << std::endl;
    arguments.serialize_callable(stream);
    stream << "    body <" << std::endl;
    body.serialize(stream, "        ");
    stream << "    >" << std::endl;
    stream << "}" << std::endl;
}

EvaluationContext Macro::evaluation_context(EvaluationResult& result) {
#if 0
    return Callable::evaluation_context(result).keeping_label_offsets();
#else
    return Entity::evaluation_context(result);
#endif
}

void Macro::resolve_implementation() { body.resolve(scope().get(), this); }
