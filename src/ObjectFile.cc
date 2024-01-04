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

#include "FileParser.h"
#include "FileReader.h"
#include <algorithm>
#include <utility>

#include "ObjectExpression.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

const Token ObjectFile::Constant::token_value{Token::NAME, "value"};

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

void ObjectFile::serialize(std::ostream& stream) const {
    stream << ".format_version " << format_version_major << "." << format_version_minor << std::endl;

    if (target && !target->name.empty()) {
        stream << ".target \"" << target->name.str() << "\"" << std::endl;
    }

    auto names = std::vector<Symbol>();
    if (!imported_libraries.empty()) {
        for (auto& library : imported_libraries) {
            names.emplace_back(library->name);
        }
        std::sort(names.begin(), names.end());
        stream << ".import ";
        auto first = true;
        for (auto name_ : names) {
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

    for (const auto& pair : pinned_objects) {
        stream << ".pin " << pair.second.name << " " << pair.second.address;
    }

    if (!explicitly_used_object_names.empty()) {
        stream << ".use";
        for (const auto& name : explicitly_used_object_names) {
            stream << " " << name;
        }
        stream << std::endl;
    }

    names.clear();
    for (const auto& pair : constants) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_ : names) {
        stream << (*constants.find(name_)->second);
    }

    names.clear();
    for (const auto& pair : functions) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_ : names) {
        stream << *(functions.find(name_)->second);
    }

    names.clear();
    for (const auto& pair : macros) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_ : names) {
        stream << (*macros.find(name_)->second);
    }

    names.clear();
    for (const auto& pair : objects) {
        names.emplace_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    for (auto name_ : names) {
        stream << *(objects.find(name_)->second);
    }
}

void ObjectFile::add_constant(std::unique_ptr<Constant> constant) {
    auto constant_name = constant->name;
    constant->set_owner(this);

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

    for (auto& pair : file->constants) {
        add_constant(std::move(pair.second));
    }
    file->constants.clear();

    for (auto& pair : file->macros) {
        add_macro(std::move(pair.second));
    }
    file->macros.clear();

    for (auto& pair : file->functions) {
        add_function(std::move(pair.second));
    }
    file->functions.clear();

    for (auto& pair : file->objects) {
        add_object(std::move(pair.second));
    }
    file->objects.clear();

    for (auto& library : file->imported_libraries) {
        import(library);
    }
}

void ObjectFile::evaluate() {
    for (auto& pair : constants) {
        EvaluationResult result;
        pair.second->value.evaluate(EvaluationContext(result, pair.second.get()));
        // TODO: process result
    }
    for (auto& pair : objects) {
        pair.second->evaluate();
    }
    for (const auto& name : explicitly_used_object_names) {
        auto it = objects.find(name);
        if (it == objects.end()) {
            FileReader::global.error({}, "explicitly used object '%s' doesn't exist", name.c_str());
            have_unresolved = true;
        }
        else {
            explicitly_used_objects.insert(it->second.get());
        }
    }
    explicitly_used_object_names.clear();

    if (!pinned_objects.empty()) {
        EvaluationResult result;
        auto context = EvaluationContext{result, EvaluationContext::STANDALONE, private_environment};
        for (auto& pair : pinned_objects) {
            auto& pin = pair.second;
            pin.address.evaluate(context);
            auto it = objects.find(pair.first);
            if (it == objects.end()) {
                FileReader::global.error({}, "pinned object '%s' doesn't exist", name.c_str());
                have_unresolved = true;
            }
            else {
                auto& pinned_object = it->second;
                if (!pin.address.has_value() || !pin.address.value()->is_unsigned()) {
                    FileReader::global.error(pin.address.location(), "pin address for '%s' must be unsigned constant", pair.first.c_str());
                }
                else if (pinned_object->address && pinned_object->address->address != pin.address.value()->unsigned_value()) {
                    FileReader::global.error(pin.address.location(), "object '%s' already has address %llu, pinned to %llu", pair.first.c_str(), pinned_object->address->address, pin.address.value()->unsigned_value());
                }
                else {
                    pinned_object->address = Address(pin.address.value()->unsigned_value());
                    explicitly_used_objects.insert(pinned_object.get());
                }
            }
        }
        if (!result.unresolved_functions.empty() || !result.unresolved_macros.empty() || !result.unresolved_variables.empty()) {
            have_unresolved = true;
        }
    }
}

void ObjectFile::remove_private_constants() {
    std::erase_if(constants, [this](const auto& item) {
        if (item.second->visibility == Visibility::PRIVATE) {
            private_environment->remove(item.first);
            return true;
        }
        return false;
    });
}

const Object* ObjectFile::object(Symbol object_name) const {
    auto it = objects.find(object_name);

    if (it != objects.end()) {
        return it->second.get();
    }
    else {
        return nullptr;
    }
}

Object* ObjectFile::create_object(Symbol section_name, Visibility visibility, const Token& object_name) {
    auto section = target->map.section(section_name);
    if (section == nullptr) {
        throw ParseException(object_name, "unknown section '%s'", section_name.c_str());
    }
    return insert_object(std::make_unique<Object>(this, section, visibility, object_name));
}

ObjectFile::ObjectFile() noexcept {
    public_environment = std::make_shared<Environment>();
    private_environment = std::make_shared<Environment>(public_environment);
}

void ObjectFile::add_to_environment(Object* object) { add_to_environment(object->name.as_symbol(), object->visibility, Expression(object)); }

void ObjectFile::add_to_environment(Symbol symbol_name, Visibility visibility, Expression value) const {
    if (visibility == Visibility::PUBLIC) {
        public_environment->add(symbol_name, std::move(value));
    }
    else {
        private_environment->add(symbol_name, std::move(value));
    }
}

std::vector<Object*> ObjectFile::all_objects() {
    std::vector<Object*> v;

    v.reserve(objects.size());
    for (auto& pair : objects) {
        v.emplace_back(pair.second.get());
    }

    return v;
}

void ObjectFile::collect_explicitly_used_objects(std::unordered_set<Object*>& set) const {
    for (auto object : explicitly_used_objects) {
        set.insert(object);
    }
    for (auto& library: imported_libraries) {
        library->collect_explicitly_used_objects(set);
    }
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
    function->set_owner(this);
    environment(function->visibility)->add(function->name.as_symbol(), function.get());
    functions[function->name.as_symbol()] = std::move(function);
}

void ObjectFile::add_macro(std::unique_ptr<Macro> macro) {
    if (macro->visibility == Visibility::SCOPE) {
        // TODO: error
        return;
    }
    macro->set_owner(this);
    environment(macro->visibility)->add(macro->name.as_symbol(), macro.get());
    macros[macro->name.as_symbol()] = std::move(macro);
}

void ObjectFile::Constant::serialize(std::ostream& stream) const {
    stream << ".constant " << name.as_symbol() << " {" << std::endl;
    serialize_entity(stream);
    stream << "    value: " << value << std::endl;
    stream << "}" << std::endl;
}

ObjectFile::Constant::Constant(ObjectFile* owner, const Token& name, const std::shared_ptr<ParsedValue>& definition) : Entity(owner, name, definition) {
    auto parameters = definition->as_dictionary();

    auto tokenizer = SequenceTokenizer((*parameters)[token_value]->as_scalar()->tokens);
    value = Expression(tokenizer);
    if (!tokenizer.ended()) {
        throw ParseException(tokenizer.current_location(), "invalid value for constant");
    }
}

void ObjectFile::import(ObjectFile* library) {
    if (imported_libraries.contains(library)) {
        return;
    }
    // TODO: detect loops
    imported_libraries.insert(library);
    private_environment->add_next(library->public_environment);
}

void ObjectFile::set_target(const Target* new_target) {
    // TODO: check compatibility
    target = new_target;
}

std::shared_ptr<Environment> ObjectFile::environment(Visibility visibility) const {
    switch (visibility) {
        case Visibility::SCOPE:
            return {}; // TODO: throw?
        case Visibility::PRIVATE:
            return private_environment;
        case Visibility::PUBLIC:
            return public_environment;
    }
}

bool ObjectFile::check_unresolved() const {
    auto ok = true;
    for (auto& pair : constants) {
        ok = pair.second->check_unresolved() && ok;
    }
    for (auto& pair : functions) {
        ok = pair.second->check_unresolved() && ok;
    }
    for (auto& pair : macros) {
        ok = pair.second->check_unresolved() && ok;
    }
    for (auto& pair : objects) {
        ok = pair.second->check_unresolved() && ok;
    }
    return ok;
}

const ObjectFile::Constant* ObjectFile::constant(Symbol name) const {
    const auto it = constants.find(name);

    if (it != constants.end()) {
        return it->second.get();
    }
    else {
        return {};
    }
}
