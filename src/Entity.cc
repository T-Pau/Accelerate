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

#include <algorithm>

#include "FileReader.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "Util.h"

#define VISIBILITY "visibility"

bool Entity::initialized = false;
Token Entity::token_visibility;

void Entity::initialize() {
    if (!initialized) {
        initialized = true;
        token_visibility = Token(Token::NAME, VISIBILITY);
        VisibilityHelper::initialize();
    }
}


Entity::Entity(ObjectFile* owner, Token name_, const std::shared_ptr<ParsedValue>& definition): environment(std::make_shared<Environment>(owner->private_environment)), owner(owner) {
    initialize();
    auto parameters = definition->as_dictionary();

    name = name_;

    auto visibility_value = (*parameters)[token_visibility]->as_singular_scalar()->token();
    auto visibility_ = VisibilityHelper::from_token(visibility_value);
    if (!visibility_) {
        throw ParseException(visibility_value, "invalid visibility '%s'", visibility_value.as_string().c_str());
    }
    visibility = *visibility_;
}

Entity::Entity(ObjectFile* owner, Token name, Visibility visibility): name(name), visibility(visibility), owner(owner), environment(std::make_shared<Environment>(owner->private_environment)) {}


void Entity::serialize_entity(std::ostream& stream) const {
    stream << "    " VISIBILITY ": " << visibility << std::endl;
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

bool Entity::check_unresolved() const {
    auto ok = check_unresolved("function", "functions", unresolved_functions);
    ok = check_unresolved("macro", "macros", unresolved_macros) && ok;
    return check_unresolved("variable", "variables", unresolved_variables) && ok;
}

bool Entity::check_unresolved(const std::string& singular, const std::string& plural, const std::unordered_set<Symbol>& unresolved) const {
    if (unresolved.empty()) {
        return true;
    }

    auto list = std::vector<Symbol>(unresolved.begin(), unresolved.end());
    if (list.size() == 1) {
        FileReader::global.error(name.location, "unresolved %s: %s", singular.c_str(), list[0].c_str());
    }
    else {
        std::sort(list.begin(), list.end());
        FileReader::global.error(name.location, "unresolved %s: %s", plural.c_str(), join(list).c_str());
    }
    return false;
}