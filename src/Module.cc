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

#include "Module.h"

Module::Module(Symbol name) : name_(name) {
    public_scope_ = std::make_shared<Scope>(Visibility::PUBLIC, name);
    private_scope_ = std::make_shared<Scope>(Visibility::PRIVATE, name, public_scope_);
}

std::shared_ptr<Scope> Module::add_file(Symbol file_name) {
    if (file_scopes.contains(file_name)) {
        throw Exception("file {} already part of module {}", file_name, name());
    }
    auto scope = std::make_shared<Scope>(Visibility::FILE, file_name, private_scope());
    file_scopes[file_name] = scope;
    return scope;
}

void Module::rename(Symbol new_name) {
    if (name_ == new_name) {
        return;
    }
    if (!public_scope_->can_rename(new_name) || !private_scope_->can_rename(new_name)) {
        throw Exception("can't rename module {} to {}: one of its scopes already contains a module with the new name", name(), new_name);
    }
    public_scope_->rename(new_name);
    private_scope_->rename(new_name);
    name_ = new_name;
}

void Module::import(Visibility visibility, const Module& module) {
    auto scope = std::shared_ptr<Scope>();

    switch (visibility) {
        case Visibility::PUBLIC:
            scope = public_scope();
            break;
        case Visibility::PRIVATE:
            scope = private_scope();
            break;
        default:
            throw Exception("cannot import module into module with visibility {}", visibility);
    }

    scope->add_next(module.public_scope());
}

std::vector<Entity*> Module::entities() const {
    std::vector<Entity*> entities;
    auto public_entities = public_scope_->get_entities();
    entities.insert(entities.end(), public_entities.begin(), public_entities.end());
    auto private_entities = private_scope_->get_entities();
    entities.insert(entities.end(), private_entities.begin(), private_entities.end());
    for (const auto& [file_name, file_scope] : file_scopes) {
        auto file_entities = file_scope->get_entities();
        entities.insert(entities.end(), file_entities.begin(), file_entities.end());
    }
    return entities;
}
