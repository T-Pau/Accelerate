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

std::ostream& operator<<(std::ostream& stream, const ObjectFile& file) {
    file.serialize(stream);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const ObjectFile::Constant& file) {
    file.serialize(stream);
    return stream;
}

void ObjectFile::serialize(std::ostream &stream) const {
    auto names = std::vector<symbol_t>();
    for (const auto& pair: constants) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end(), SymbolTable::global_less);

    for (auto name: names) {
        stream << constants.find(name)->second;
    }

    names.clear();
    for (const auto& pair: objects) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end(), SymbolTable::global_less);

    for (auto name: names) {
        stream << objects.find(name)->second;
    }
}

void ObjectFile::add_constant(symbol_t name, Object::Visibility visibility, std::shared_ptr<Expression> value) {
    auto pair = constants.insert({name, Constant(name, visibility, std::move(value))});

    if (!pair.second) {
        throw ParseException(Location(), "duplicate symbol");
    }

    add_to_environment(pair.first->second);
}

void ObjectFile::add_object_file(const ObjectFile &file) {
    for (const auto& pair: file.constants) {
        add_constant(pair.second.name, pair.second.visibility, pair.second.value);
    }
    for (const auto& pair: file.objects) {
        auto foo = objects.insert(pair);
        if (!foo.second) {
            throw ParseException(Location(), "duplicate object");
        }
        add_to_environment(&foo.first->second);
    }
}

void ObjectFile::evaluate(const Environment &environment) {
    for (auto& pair: constants) {
        pair.second.value = Expression::evaluate(pair.second.value, environment);
    }
    for (auto& pair: objects) {
        pair.second.data.evaluate(environment);
    }
}


void ObjectFile::export_constants(Environment &environment) const {
    for (auto& pair: constants) {
        environment.add(pair.second.name, pair.second.value);
    }
}


void ObjectFile::remove_local_constants() {
    std::erase_if(constants, [this](const auto& item) {
        if (item.second.visibility == Object::LOCAL) {
            local_environment->remove(item.first);
            return true;
        }
        return false;
    });
}


const Object* ObjectFile::object(symbol_t name) const {
    auto it = objects.find(name);

    if (it != objects.end()) {
        return &it-> second;
    }
    else {
        return nullptr;
    }
}


Object *ObjectFile::create_object(symbol_t section, Object::Visibility visibility, Token name) {
    auto pair = objects.insert({name.as_symbol(), {this, section, visibility, name}});
    if (!pair.second) {
        throw ParseException(name, "redefinition of object");
    }
    return &pair.first->second;
}

ObjectFile::ObjectFile() {
    global_environment = std::make_shared<Environment>();
    local_environment = std::make_shared<Environment>(global_environment);
}

void ObjectFile::add_to_environment(Object *object) {
    add_to_environment(object->name.as_symbol(), object->visibility, std::make_shared<ObjectExpression>(object));
}

void ObjectFile::add_to_environment(symbol_t name, Object::Visibility visibility, std::shared_ptr<Expression> value) {
    if (visibility == Object::GLOBAL) {
        global_environment->add(name, std::move(value));
    }
    else {
        local_environment->add(name, std::move(value));
    }
}

std::vector<Object *> ObjectFile::all_objects() {
    std::vector<Object*> v;

    for (auto& pair: objects) {
        v.emplace_back(&pair.second);
    }

    return v;
}


void ObjectFile::Constant::serialize(std::ostream &stream) const {
    stream << ".constant " << SymbolTable::global[name] << " {" << std::endl;
    stream << "    visibility: " << visibility << std::endl;
    stream << "    value: " << value << std::endl;
    stream << "}" << std::endl;
}
