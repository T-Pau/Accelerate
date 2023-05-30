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

#include <utility>

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
        stream << constants.find(name_)->second;
    }

    names.clear();
    for (const auto& pair: functions) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_: names) {
        stream << functions.find(name_)->second;
    }

    names.clear();
    for (const auto& pair: macros) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_: names) {
        stream << macros.find(name_)->second;
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

void ObjectFile::add_constant(Symbol symbol_name, Visibility visibility, Expression value) {
    auto pair = constants.insert({symbol_name, Constant(symbol_name, visibility, std::move(value))});

    if (!pair.second) {
        throw ParseException(Location(), "duplicate symbol '%s'", symbol_name.c_str());
    }

    add_to_environment(pair.first->second);
}

void ObjectFile::add_object_file(const std::shared_ptr<ObjectFile>& file) {
    if (file->target) {
        if (target && !target->name.empty() && target != file->target) {
            throw Exception("can't combine files for different targets");
        }
        target = file->target;
    }

    for (const auto& pair: file->constants) {
        add_constant(pair.second.name, pair.second.visibility, pair.second.value);
    }

    for (auto& pair: file->macros) {
        add_macro(std::move(pair.second));
    }
    file->functions.clear();

    for (auto& pair: file->functions) {
        add_function(std::move(pair.second));
    }
    file->functions.clear();

    for (auto& pair: file->objects) {
        pair.second->environment->replace(pair.second->owner->local_environment, local_environment);
        add_object(std::move(pair.second));
    }
    file->objects.clear();

    for (auto& library: file->imported_libraries) {
        import(library);
    }
}


void ObjectFile::evaluate() {
    for (auto& pair: constants) {
        pair.second.value.evaluate(local_environment);
    }
    for (auto& pair: objects) {
        pair.second->evaluate();
    }
}


void ObjectFile::evaluate(const std::shared_ptr<Environment>& environment) {
    for (auto& pair: constants) {
        pair.second.value.evaluate(environment);
    }
    for (auto& pair: objects) {
        pair.second->body.evaluate(pair.second->name.as_symbol(), this, environment);
    }
}


void ObjectFile::remove_local_constants() {
    std::erase_if(constants, [this](const auto& item) {
        if (item.second.visibility == Visibility::LOCAL) {
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
    auto object_name = object->name;
    object->owner = this;
    auto pair = objects.insert({object_name.as_symbol(), std::move(object)});
    if (!pair.second) {
        throw ParseException(object_name, "redefinition of object %s", object_name.as_string().c_str());
    }
    auto own_object = pair.first->second.get();
    add_to_environment(own_object);
    return own_object;

}

void ObjectFile::insert_function(std::unique_ptr<Function> function) {
    functions[function->name.as_symbol()] = std::move(function);
    // TODO: add to environment
}

void ObjectFile::insert_macro(std::unique_ptr<Macro> macro) {
    macros[macro->name.as_symbol()] = std::move(macro);
    // TODO: add to environment
}

void ObjectFile::Constant::serialize(std::ostream &stream) const {
    stream << ".constant " << name << " {" << std::endl;
    stream << "    visibility: " << visibility << std::endl;
    stream << "    value: " << value << std::endl;
    stream << "}" << std::endl;
}


void ObjectFile::import(ObjectFile *library) {
    if (imported_libraries.contains(library)) {
        return;
    }
    // TODO: detect loops
    imported_libraries.insert(library);
    global_environment->add_next(library->global_environment);
}
