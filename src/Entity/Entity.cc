/*
Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "Entity/Entity.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/LocationException.h>

#include "Entity/Constant.h"
#include "Entity/Function.h"
#include "Entity/Macro.h"
#include "Entity/Object.h"
#include "ExpressionParser.h"
#include "Scope.h"
#include "SequenceTokenizer.h"
#include "StructuredDictionary.h"
#include "StructuredScalar.h"

using namespace tpau::cpp_kernal;

#define DEFAULT_ONLY "default_only"
#define VISIBILITY "visibility"

const Token Entity::token_default_only = Token(Token::NAME, DEFAULT_ONLY);
const Token Entity::token_visibility = Token(Token::NAME, VISIBILITY);

Entity::Entity(const Location& location, Symbol name, std::shared_ptr<Scope> containing_scope, const std::shared_ptr<StructuredValue>& definition) : name(name), location(location), containing_scope_(containing_scope) {
    const auto parameters = definition->as_dictionary();

    if (const auto default_only_definition = parameters->get_optional(token_default_only)) {
        auto tokenizer = SequenceTokenizer(default_only_definition->as_scalar()->tokens);
        auto default_only_value = ExpressionParser(tokenizer).parse();
        if (!default_only_value.has_value() || !default_only_value.value()->is_boolean()) {
            throw LocationException(default_only_definition->location, "invalid default_only");
        }
        default_only = default_only_value.value()->boolean_value();
    }

    const auto visibility_value = (*parameters)[token_visibility]->as_singular_scalar()->token();
    const auto visibility_ = VisibilityHelper::from_token(visibility_value);
    if (!visibility_) {
        throw LocationException(visibility_value.location, "invalid visibility '{}'", visibility_value);
    }
    visibility = *visibility_;
}

void Entity::serialize_entity(std::ostream& stream) const {
    stream << "    " VISIBILITY ": " << visibility << std::endl;
    if (default_only) {
        stream << "    " DEFAULT_ONLY ": .true" << std::endl;
    }
}

void Entity::evaluate() {
    TRACE_BEGIN("evaluating", "{}", name);
    auto context = evaluation_context();
    try {
        evaluate_implementation(context);
    }
    catch (Exception& ex) {
        DiagnosticOutput::global.error(location, ex);
        // TODO: throw empty expression?
    }
    TRACE_END("evaluating", "{}", name);
}

void Entity::uses(Entity* entity) {
    if (entity->visibility <= Visibility::ENTITY || (entity == this && is<Object>())) {
        // We don't track references to entity only visible within an entity.
        // Objects refer to themselves for their address via label expressions.
        return;
    }
    referenced_entities.insert(entity);
}

void Entity::resolve() {
    if (resolved) {
        return;
    }
    resolved = true;

    TRACE_BEGIN("resolving", "{}", name);
    DiagnosticOutput::global.log_exceptions([this]() { resolve_implementation(); });
    TRACE_END("resolving", "{}", name);
}

std::ostream& operator<<(std::ostream& stream, const Entity& entity) {
    entity.serialize(stream);
    return stream;
}

std::shared_ptr<Scope> Entity::containing_scope() const {
    auto scope = containing_scope_.lock();
    if (!scope) {
        throw LocationException(location, "internal error: containing scope of {} '{}' destroyed", type_name(), name);
    }
    return scope;
}

void Entity::expand_calls() {
    TRACE_BEGIN("expanding calls", "{}", name);
    DiagnosticOutput::global.log_exceptions([this]() { expand_calls_implementation(); });
    TRACE_END("expanding calls", "{}", name);
}

void Entity::expand_calls_implementation() {
    traverse([](Entity& entity) { entity.expand_calls(); }, [](Body& body) { body.expand_calls(); }, [](Expression& expression) { expression.expand_calls(); });
}

void Entity::resolve_implementation() {
    traverse([this](Entity& entity) { entity.resolve(); }, [this](Body& body) { body.resolve(containing_scope().get(), this); }, [this](Expression& expression) { expression.resolve(containing_scope().get(), this); });
}

void Entity::evaluate_implementation(EvaluationContext& context) {
    traverse([](Entity& entity) { entity.evaluate(); }, [&context](Body& body) { body.evaluate(context); }, [&context](Expression& expression) { expression.evaluate(context); });
    evaluate_process(context);
}
