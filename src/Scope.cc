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

#include "Scope.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/Exception.h>

#include "Entity/Constant.h"
#include "Entity/Function.h"
#include "Entity/Macro.h"
#include "Entity/Object.h"
#include "Module.h"

using namespace tpau::cpp_kernal;

Scope::Scope(Visibility type, Symbol name) : type_(type), name(name) {}

Scope::Scope(Visibility type, Symbol name, std::shared_ptr<Scope> next) : type_(type), name(name), next({std::move(next)}) {
    if (!can_contain(type_, next->type())) {
        throw Exception("cannot add scope of type {} to scope of type {}", type_, next->type());
    }
}

bool Scope::is_defined(Symbol name) const { // NOLINT(misc-no-recursion)
    if (defines.contains(name)) {
        return true;
    }
    if (undefine_overrides.contains(name)) {
        return false;
    }
    for (auto& environment : next) {
        if (environment->is_defined(name)) {
            return true;
        }
    }
    return false;
}

bool Scope::can_contain(Visibility container, Visibility contained) {
    switch (contained) {
        case Visibility::SCOPE:
            return container == Visibility::SCOPE || container == Visibility::ENTITY;
        case Visibility::ENTITY:
            return container == Visibility::FILE;
        case Visibility::FILE:
            return container == Visibility::PRIVATE;
        case Visibility::PRIVATE:
            return container == Visibility::PUBLIC;
        default:
            return false;
    }
}

Scope* Scope::find_containing_scope(Visibility visibility) {
    Scope* scope = this;

    while (true) {
        if (scope->type() == visibility) {
            return scope;
        }
        if (scope->next.empty()) {
            throw Exception("cannot find containing scope of type {}", visibility);
        }
        scope = scope->next.front().get();
    }
}

std::optional<Location> Scope::find_conflicting_constant_or_object(Symbol name) {
    if (auto constant = get_constant(name)) {
        return constant->location;
    }
    if (auto object = get_object(name)) {
        return object->location;
    }
    return {};
}

std::optional<Location> Scope::find_conflicting_function(Symbol name) {
    if (auto function = get_function(name)) {
        return function->location;
    }
    return {};
}

std::optional<Location> Scope::find_conflicting_macro(Symbol name) {
    if (auto macro = get_macro(name)) {
        return macro->location;
    }
    return {};
}

void Scope::add_next(std::shared_ptr<Scope> scope) {
    if (!can_contain(scope->type(), type())) {
        throw Exception("cannot add scope of type {} to scope of type {}", type(), scope->type());
    }
    next.push_back(std::move(scope));
}

void Scope::add(std::unique_ptr<Object> object) {
    auto scope = add_precheck<Object>(object->visibility, object->name, object->location);
    scope->objects[object->name] = std::move(object);
}

void Scope::add(std::unique_ptr<Constant> constant) {
    auto scope = add_precheck<Constant>(constant->visibility, constant->name, constant->location);
    scope->constants[constant->name] = std::move(constant);
}

void Scope::add(std::unique_ptr<Function> function) {
    auto scope = add_precheck<Function>(function->visibility, function->name, function->location);
    scope->functions[function->name] = std::move(function);
}

void Scope::add(std::unique_ptr<Macro> macro) {
    auto scope = add_precheck<Macro>(macro->visibility, macro->name, macro->location);
    scope->macros[macro->name] = std::move(macro);
}

void Scope::import(Visibility visibility, const Module& module) {
    if (visibility == Visibility::SCOPE) {
        throw Exception("cannot import module into scope with visibility {}", visibility);
    }
    auto scope = find_containing_scope(visibility);
    scope->add_next(module.public_scope());
}
