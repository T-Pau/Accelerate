/*
Entity.cc --

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

#include "Entity.h"

#include "ExpressionParser.h"
#include "FileReader.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

#define DEFAULT_ONLY "default_only"
#define VISIBILITY "visibility"

const Token Entity::token_default_only = Token(Token::NAME, DEFAULT_ONLY);
const Token Entity::token_visibility = Token(Token::NAME, VISIBILITY);


Entity::Entity(ObjectFile* owner, const Token& name_, const std::shared_ptr<ParsedValue>& definition): owner(owner), environment(std::make_shared<Environment>(owner->private_environment)) {
    const auto parameters = definition->as_dictionary();

    name = name_;

    if (const auto default_only_definition = parameters->get_optional(token_default_only)) {
        auto tokenizer = SequenceTokenizer(default_only_definition->as_scalar()->tokens);
        auto default_only_value = ExpressionParser(tokenizer).parse();
        if (!default_only_value.has_value() || !default_only_value.value()->is_boolean()) {
            throw ParseException(default_only_definition->location, "invalid default_only");
        }
        default_only = default_only_value.value()->boolean_value();
    }

    const auto visibility_value = (*parameters)[token_visibility]->as_singular_scalar()->token();
    const auto visibility_ = VisibilityHelper::from_token(visibility_value);
    if (!visibility_) {
        throw ParseException(visibility_value, "invalid visibility '%s'", visibility_value.as_string().c_str());
    }
    visibility = *visibility_;
}

Entity::Entity(ObjectFile* owner, const Token& name, Visibility visibility, bool default_only): name(name), visibility(visibility), owner(owner), environment(std::make_shared<Environment>(owner->private_environment)), default_only{default_only} {}


void Entity::serialize_entity(std::ostream& stream) const {
    stream << "    " VISIBILITY ": " << visibility << std::endl;
    if (default_only) {
        stream << "    " DEFAULT_ONLY ": .true" << std::endl;
    }
}

void Entity::set_owner(ObjectFile* new_owner) {
    if (new_owner == owner) {
        return;
    }
    if (owner) {
        environment->replace(owner->private_environment, new_owner->private_environment);
    }
    owner = new_owner;
}

Macro* Entity::as_macro() {
    return dynamic_cast<Macro*>(this);
}

Object* Entity::as_object() {
    return dynamic_cast<Object*>(this);
}

void Entity::process_result(EvaluationResult& result) {
    referenced_objects.insert(result.used_objects.begin(), result.used_objects.end());
    unresolved_functions = std::move(result.unresolved_functions);
    unresolved_macros = std::move(result.unresolved_macros);
    unresolved_variables = std::move(result.unresolved_variables);
}

bool Entity::check_unresolved(Unresolved& unresolved) const {
    auto ok = check_unresolved(unresolved_functions, unresolved.functions);
    ok = check_unresolved(unresolved_macros, unresolved.macros) && ok;
    return check_unresolved(unresolved_variables, unresolved.variables) && ok;
}

bool Entity::check_unresolved(const std::unordered_set<Symbol>& unresolved, Unresolved::Part& part) const {
    if (unresolved.empty()) {
        return true;
    }

    for (auto& unresolved_name: unresolved) {
        part.add(name, unresolved_name);
    }
    return false;
}


void Entity::evaluate() {
    auto result = EvaluationResult{};
    auto context = evaluation_context(result);
    try {
        evaluate_inner(context);
        process_result(result);
    }
    catch (Exception &ex) {
        FileReader::global.error(ParseException(name.location, ex));
        // TODO: throw empty expression?
    }
}

void Entity::resolve_labels() {
    try {
        auto result = EvaluationResult{};
        auto context = evaluation_context(result);
        context.type = EvaluationContext::LABELS;
        evaluate_inner(context);

        result = EvaluationResult{};
        auto context2 = evaluation_context(result);
        context2.type = EvaluationContext::LABELS_2;
        evaluate_inner(context2);
    }
    catch (Exception &ex) {
        FileReader::global.error(ParseException(name.location, ex));
        // TODO: throw empty expression?
    }
}