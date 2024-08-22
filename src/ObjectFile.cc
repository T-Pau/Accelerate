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

#include <algorithm>
#include <ranges>

#include "FileReader.h"
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
        std::ranges::sort(names);
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

    for (const auto& pinned_object : pinned_objects | std::views::values) {
        stream << ".pin " << pinned_object.name << " " << pinned_object.address;
    }

    if (!explicitly_used_object_names.empty()) {
        stream << ".use";
        for (const auto& name : explicitly_used_object_names) {
            stream << " " << name;
        }
        stream << std::endl;
    }

    names.clear();
    for (const auto& name: constants | std::views::keys) {
        names.emplace_back(name);
    }
    std::ranges::sort(names);
    for (auto name_ : names) {
        stream << (*constants.find(name_)->second);
    }

    names.clear();
    for (const auto& name: functions | std::views::keys) {
        names.emplace_back(name);
    }
    std::ranges::sort(names);
    for (auto name_ : names) {
        stream << *(functions.find(name_)->second);
    }

    names.clear();
    for (const auto& name: macros | std::views::keys) {
        names.emplace_back(name);
    }
    std::ranges::sort(names);
    for (auto name_ : names) {
        stream << (*macros.find(name_)->second);
    }

    names.clear();
    for (const auto& name: objects | std::views::keys) {
        names.emplace_back(name);
    }
    std::ranges::sort(names);
    for (auto name_ : names) {
        stream << *(objects.find(name_)->second);
    }
}

void ObjectFile::add_constant(std::unique_ptr<Constant> constant) {
    auto own_constant = constant.get();
    auto it = constants.find(constant->name);
    if (it != constants.end()) {
        if (constant->is_default_only()) {
            return;
        }
        throw ParseException(constant->location, "redefinition of constant '%s'", constant->name.c_str()).appending(it->second->location, "previously defined here");
    }

    constant->set_owner(this);
    constants[constant->name] = std::move(constant);
    if (!own_constant->is_default_only()) {
        add_to_environment(own_constant->name, own_constant->visibility, own_constant->value);
    }
}

void ObjectFile::add_object_file(const std::shared_ptr<ObjectFile>& file) {
    auto ok = true;

    if (file->target) {
        if (target && !target->name.empty() && target != file->target) {
            throw Exception("can't combine files for different targets");
        }
        target = file->target;
    }

    for (auto& constant : file->constants | std::views::values) {
        try {
            add_constant(std::move(constant));
        } catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }
    file->constants.clear();

    for (auto& macro : file->macros | std::views::values) {
        try {
            add_macro(std::move(macro));
        } catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }
    file->macros.clear();

    for (auto& function : file->functions | std::views::values) {
        try {
            add_function(std::move(function));
        } catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }
    file->functions.clear();

    for (auto& object : file->objects | std::views::values) {
        try {
            add_object(std::move(object));
        } catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }
    file->objects.clear();

    for (auto& library : file->imported_libraries) {
        try {
            import(library);
        } catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }

    explicitly_used_object_names.insert(file->explicitly_used_object_names.begin(), explicitly_used_object_names.end());
    file->explicitly_used_object_names.clear();
    explicitly_used_objects.insert(file->explicitly_used_objects.begin(), explicitly_used_objects.end());
    file->explicitly_used_objects.clear();
    for (const auto& pair : file->pinned_objects) {
        try {
            pin(pair.first, pair.second.address);
        } catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }
    file->pinned_objects.clear();

    if (!ok) {
        throw Exception();
    }
}

void ObjectFile::evaluate() {
    for (auto& constant : constants | std::views::values) {
        // TODO: this causes the error for cyclic definition to be printed four times
        //constant->evaluate();
        EvaluationResult result;
        constant->value.evaluate(EvaluationContext(result, constant.get()));
        constant->process_result(result);
    }
    for (auto& object : objects | std::views::values) {
        object->evaluate();
    }
    for (auto& function : functions | std::views::values) {
        function->evaluate();
    }
    for (auto& macro : macros | std::views::values) {
        macro->evaluate();
    }

    unresolved_used.clear();
    auto new_explicitly_used_objects = std::unordered_set<Symbol>{};
    for (const auto& symbol : explicitly_used_object_names) {
        auto it = objects.find(symbol);
        if (it != objects.end()) {
            explicitly_used_objects.insert(it->second.get());
            new_explicitly_used_objects.insert(symbol);
        }
        else {
            unresolved_used.variables.add(it->second.get(), symbol);
        }
    }
    explicitly_used_object_names = new_explicitly_used_objects;

    if (!pinned_objects.empty()) {
        auto new_pinned_objects = std::unordered_map<Symbol, Pinned>{};

        EvaluationResult result;
        auto context = EvaluationContext{result, EvaluationContext::STANDALONE, private_environment};
        for (auto& pair : pinned_objects) {
            auto& pin = pair.second;
            pin.address.evaluate(context);
            auto object_expression = private_environment->get_variable(pair.first);
            if (object_expression && object_expression->is_object()) {
                auto pinned_object = object_expression->as_object()->object;
                if (!pin.address.has_value()) {
                    unresolved_used.variables.add({}, pair.first);
                    new_pinned_objects[pair.first] = pair.second;
                }
                else if (!pin.address.value()->is_unsigned()) {
                    FileReader::global.error(pin.address.location(), "pin address for '%s' must be unsigned constant", pair.first.c_str());
                }
                else if (pinned_object->address && pinned_object->address->address != pin.address.value()->unsigned_value()) {
                    FileReader::global.error(pin.address.location(), "object '%s' already has address %llu, pinned to %llu", pair.first.c_str(), pinned_object->address->address, pin.address.value()->unsigned_value());
                }
                else {
                    pinned_object->address = Address(pin.address.value()->unsigned_value());
                    explicitly_used_objects.insert(pinned_object);
                }
            }
            else {
                unresolved_used.variables.add(name, {}, pair.first);
                new_pinned_objects[pair.first] = pair.second;
            }
        }
        pinned_objects = new_pinned_objects;
        // TODO: add locations of .pin directives.
        unresolved_used.add(name, {}, result);
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

void ObjectFile::resolve_defaults() {
    for (const auto& [name, constant] : constants) {
        if (constant->is_default_only() && !private_environment->get_variable(name)) {
            add_to_environment(constant->name, constant->visibility, constant->value);
        }
    }

    for (const auto& [name, function] : functions) {
        if (function->is_default_only() && !private_environment->get_function(name)) {
            environment(function->visibility)->add(function->name, function.get());
        }
    }

    for (const auto& [name, macro] : macros) {
        if (macro->is_default_only() && !private_environment->get_macro(name)) {
            environment(macro->visibility)->add(macro->name, macro.get());
        }
    }

    for (const auto& [name, object] : objects) {
        if (object->is_default_only() && !private_environment->get_variable(name)) {
            add_to_environment(object.get());
        }
    }

    unused_default_objects.clear();
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

Object* ObjectFile::create_object(Symbol section_name, Visibility visibility, bool default_only, const Token& object_name) {
    auto section = target->map.section(section_name);
    if (section == nullptr) {
        if (section_name.empty()) {
            FileReader::global.error(object_name.location, "object outside section");
        }
        else {
            FileReader::global.error(object_name.location, "unknown section '%s'", section_name.c_str());
        }
        // TODO: mark object as faulty
    }
    return insert_object(std::make_unique<Object>(this, section, visibility, default_only, object_name));
}

ObjectFile::ObjectFile() noexcept {
    public_environment = std::make_shared<Environment>();
    private_environment = std::make_shared<Environment>(public_environment);
}

void ObjectFile::add_to_environment(Object* object) { add_to_environment(object->name, object->visibility, Expression(object->location, object)); }

void ObjectFile::add_to_environment(Symbol symbol_name, Visibility visibility, Expression value) const {
    if (visibility == Visibility::PUBLIC) {
        public_environment->add(symbol_name, std::move(value));
    }
    else {
        private_environment->add(symbol_name, std::move(value));
    }
}

std::vector<Object*> ObjectFile::all_objects() const {
    std::vector<Object*> v;

    v.reserve(objects.size());
    for (auto& object : objects | std::views::values) {
        v.emplace_back(object.get());
    }

    return v;
}

void ObjectFile::collect_explicitly_used_objects(std::unordered_set<Object*>& set) const { // NOLINT(misc-no-recursion)
    for (auto object : explicitly_used_objects) {
        set.insert(object);
    }
    for (auto& library : imported_libraries) {
        library->collect_explicitly_used_objects(set);
    }
}

Object* ObjectFile::insert_object(std::unique_ptr<Object> object) {
    auto [it, inserted] = objects.insert({object->name, std::move(object)});
    if (inserted) {
        auto own_object = it->second.get();
        own_object->set_owner(this);
        add_to_environment(own_object);
        return own_object;
    }
    else {
        if (object->is_default_only()) {
            object->set_owner(this);
            auto [defaults_it, _] = unused_default_objects.insert(std::move(object));
            return defaults_it->get();
        }
        throw ParseException(object->location, "redefinition of object '%s'", object->name.c_str()).appending(it->second->location, "previously defined here");
    }
}

void ObjectFile::add_function(std::unique_ptr<Function> function) {
    if (function->visibility == Visibility::SCOPE) {
        // TODO: error
        return;
    }

    auto it = functions.find(function->name);
    if (it != functions.end()) {
        if (function->is_default_only()) {
            return;
        }
        else {
            throw ParseException(function->location, "redefinition of function %s", function->name.c_str());
        }
    }

    function->set_owner(this);
    if (!function->is_default_only()) {
        environment(function->visibility)->add(function->name, function.get());
    }
    functions[function->name] = std::move(function);
}

void ObjectFile::add_macro(std::unique_ptr<Macro> macro) {
    if (macro->visibility == Visibility::SCOPE) {
        // TODO: error
        return;
    }

    auto it = macros.find(macro->name);
    if (it != macros.end()) {
        if (macro->is_default_only()) {
            return;
        }
        else {
            throw ParseException(macro->location, "redefinition of macro %s", macro->name.c_str());
        }
    }

    macro->set_owner(this);
    if (!macro->is_default_only()) {
        environment(macro->visibility)->add(macro->name, macro.get());
    }
    macros[macro->name] = std::move(macro);
}

void ObjectFile::Constant::serialize(std::ostream& stream) const {
    stream << ".constant " << name << " {" << std::endl;
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

    throw Exception("internal error: invalid visibility");
}

bool ObjectFile::check_unresolved(Unresolved& unresolved) const {
    auto ok = true;
    for (auto& constant : constants | std::views::values) {
        ok = constant->check_unresolved(unresolved) && ok;
    }
    for (auto& function : functions | std::views::values) {
        ok = function->check_unresolved(unresolved) && ok;
    }
    for (auto& macro : macros | std::views::values) {
        ok = macro->check_unresolved(unresolved) && ok;
    }
    for (auto& object : objects | std::views::values) {
        ok = object->check_unresolved(unresolved) && ok;
    }
    if (!unresolved_used.empty()) {
        unresolved.add(unresolved_used);
        ok = false;
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

Macro* ObjectFile::macro(Symbol macro_name) {
    auto it = macros.find(macro_name);
    if (it != macros.end()) {
        return it->second.get();
    }
    else {
        return nullptr;
    }
}


std::unordered_set<Entity*> ObjectFile::public_entities() const {
    auto entities = std::unordered_set<Entity*>{};

    for (auto& constant: constants | std::views::values) {
        if (constant->is_public()) {
            entities.insert(constant.get());
        }
    }

    for (auto& function: functions | std::views::values) {
        if (function->is_public()) {
            entities.insert(function.get());
        }
    }

    for (auto& macro: macros | std::views::values) {
        if (macro->is_public()) {
            entities.insert(macro.get());
        }
    }

    for (auto& object: objects | std::views::values) {
        if (object->is_public()) {
            entities.insert(object.get());
        }
    }

    entities.insert(explicitly_used_objects.begin(), explicitly_used_objects.end());

    return entities;
}