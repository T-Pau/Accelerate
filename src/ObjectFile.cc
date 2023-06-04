/*
ObjectFile.cc --

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

#include "ObjectFile.h"
#include "ParseException.h"
#include "ObjectExpression.h"
#include "SequenceTokenizer.h"

#include <utility>

bool ObjectFile::Constant::initialized = false;
Token ObjectFile::Constant::token_value;

void ObjectFile::Constant::initialize() {
    if (!initialized) {
        initialized = true;
        token_value = Token(Token::NAME, "value");
    }
}

const unsigned int ObjectFile::format_version_major = 1;
const unsigned int ObjectFile::format_version_minor = 0;

std::ostream& operator<<(std::ostream& stream, const ObjectFile& file) {
    file.serialize(stream);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<ObjectFile>& file) {
    file->serialize(stream);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const ObjectFile::Constant& file) {
    file.serialize(stream);
    return stream;
}

void ObjectFile::serialize(std::ostream &stream) const {
    stream << ".format_version " << format_version_major << "." << format_version_minor << std::endl;

    if (target && !target->name.empty()) {
        stream << ".target \"" << target->name.str() << "\"" << std::endl;
    }

    auto names = std::vector<Symbol>();
    if (!imported_libraries.empty()) {
        for (auto& library: imported_libraries) {
            names.emplace_back(library->name);
        }
        std::sort(names.begin(), names.end());
        stream << ".import ";
        auto first = true;
        for (auto name_: names) {
            if (first) {
                first = false;
            }
            else {
                stream << ", ";
            }
            stream << "\"" << name_ << "\"";
        }
        stream << std::endl;
    }

    names = std::vector<Symbol>();
    for (const auto& pair: constants) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_ : names) {
        stream << (*constants.find(name_)->second);
    }

    names.clear();
    for (const auto& pair: functions) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_: names) {
        stream << *(functions.find(name_)->second);
    }

    names.clear();
    for (const auto& pair: macros) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_: names) {
        stream << (*macros.find(name_)->second);
    }

    names.clear();
    for (const auto& pair: objects) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_: names) {
        stream << *(objects.find(name_)->second);
    }
}

void ObjectFile::add_constant(std::unique_ptr<Constant> constant) {
    auto constant_name = constant->name;

    auto pair = constants.insert({constant_name.as_symbol(), std::move(constant)});

    if (!pair.second) {
        throw ParseException(Location(), "duplicate symbol '%s'", constant_name.as_string().c_str());
    }

    add_to_environment(pair.first->second->name.as_symbol(), pair.first->second->visibility, pair.first->second->value);
}


void ObjectFile::add_object_file(const std::shared_ptr<ObjectFile>& file) {
    if (file->target) {
        if (target && !target->name.empty() && target != file->target) {
            throw Exception("can't combine files for different targets");
        }
        target = file->target;
    }

    for (auto& pair: file->constants) {
        add_constant(std::move(pair.second));
    }
    file->constants.clear();

    for (auto& pair: file->macros) {
        add_macro(std::move(pair.second));
    }
    file->macros.clear();

    for (auto& pair: file->functions) {
        add_function(std::move(pair.second));
    }
    file->functions.clear();

    for (auto& pair: file->objects) {
        add_object(std::move(pair.second));
    }
    file->objects.clear();

    for (auto& library: file->imported_libraries) {
        import(library);
    }
}


void ObjectFile::evaluate() {
    for (auto& pair: constants) {
        pair.second->value.evaluate(local_environment);
    }
    for (auto& pair: objects) {
        pair.second->evaluate();
    }
}


void ObjectFile::evaluate(const std::shared_ptr<Environment>& environment) {
    for (auto& pair: constants) {
        pair.second->value.evaluate(environment);
    }
    for (auto& pair: objects) {
        pair.second->body.evaluate(pair.second.get(), environment);
    }
}


void ObjectFile::remove_local_constants() {
    std::erase_if(constants, [this](const auto& item) {
        if (item.second->visibility == Visibility::LOCAL) {
            local_environment->remove(item.first);
            return true;
        }
        return false;
    });
}


const Object* ObjectFile::object(Symbol object_name) const {
    auto it = objects.find(object_name);

    if (it != objects.end()) {
        return it-> second.get();
    }
    else {
        return nullptr;
    }
}


Object *ObjectFile::create_object(Symbol section_name, Visibility visibility, Token object_name) {
    auto section = target->map.section(section_name);
    if (section == nullptr) {
        throw ParseException(object_name, "unknown section '%s'", section_name.c_str());
    }
    return insert_object(std::make_unique<Object>(this, section, visibility, object_name));
}

ObjectFile::ObjectFile() noexcept {
    global_environment = std::make_shared<Environment>();
    local_environment = std::make_shared<Environment>(global_environment);
}

void ObjectFile::add_to_environment(Object *object) {
    add_to_environment(object->name.as_symbol(), object->visibility, Expression(object));
}

void ObjectFile::add_to_environment(Symbol symbol_name, Visibility visibility, Expression value) const {
    if (visibility == Visibility::GLOBAL) {
        global_environment->add(symbol_name, std::move(value));
    }
    else {
        local_environment->add(symbol_name, std::move(value));
    }
}

std::vector<Object *> ObjectFile::all_objects() {
    std::vector<Object*> v;

    v.reserve(objects.size());
    for (auto& pair: objects) {
        v.emplace_back(pair.second.get());
    }

    return v;
}

Object* ObjectFile::insert_object(std::unique_ptr<Object> object) {
    auto own_object = object.get();
    auto pair = objects.insert({own_object->name.as_symbol(), std::move(object)});
    if (!pair.second) {
        throw ParseException(own_object->name, "redefinition of object %s", own_object->name.as_string().c_str());
    }
    own_object->set_owner(this);
    add_to_environment(own_object);
    return own_object;

}

void ObjectFile::add_function(std::unique_ptr<Function> function) {
    if (function->visibility == Visibility::SCOPE) {
        // TODO: error
        return;
    }
    environment(function->visibility)->add(function->name.as_symbol(), function.get());
    functions[function->name.as_symbol()] = std::move(function);
}

void ObjectFile::add_macro(std::unique_ptr<Macro> macro) {
    if (macro->visibility == Visibility::SCOPE) {
        // TODO: error
        return;
    }
    environment(macro->visibility)->add(macro->name.as_symbol(), macro.get());
    macros[macro->name.as_symbol()] = std::move(macro);
}

void ObjectFile::Constant::serialize(std::ostream &stream) const {
    stream << ".constant " << name.as_symbol() << " {" << std::endl;
    serialize_entity(stream);
    stream << "    value: " << value << std::endl;
    stream << "}" << std::endl;
}

ObjectFile::Constant::Constant(Token name, const std::shared_ptr<ParsedValue>& definition): Entity(name, definition) {
    initialize();

    auto parameters = definition->as_dictionary();

    auto tokenizer = SequenceTokenizer((*parameters)[token_value]->as_scalar()->tokens);
    value = Expression(tokenizer);
    if (!tokenizer.ended()) {
        throw ParseException(tokenizer.current_location(), "invalid value for constant");
    }
}

void ObjectFile::import(ObjectFile *library) {
    if (imported_libraries.contains(library)) {
        return;
    }
    // TODO: detect loops
    imported_libraries.insert(library);
    local_environment->add_next(library->global_environment);
}

void ObjectFile::set_target(const Target* new_target) {
    // TODO: check compatibility
    target = new_target;
}

std::shared_ptr<Environment> ObjectFile::environment(Visibility visibility) const {
    switch (visibility) {
        case Visibility::SCOPE:
            return {}; // TODO: throw?
        case Visibility::LOCAL:
            return local_environment;
        case Visibility::GLOBAL:
            return global_environment;
    }
}