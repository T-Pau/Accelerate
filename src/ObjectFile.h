/*
ObjectFile.h --

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

#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#include <map>

#include "Function.h"
#include "Macro.h"
#include "Object.h"
#include "Symbol.h"
#include "Target.h"

class ObjectFile {
public:
    class Constant: public Entity {
    public:
        Constant(ObjectFile* owner, Token name, Visibility visibility, Expression value): Entity(owner, name, visibility), value(std::move(value)) {}
        Constant(ObjectFile* owner, const Token& name, const std::shared_ptr<ParsedValue>& definition);

        void serialize(std::ostream& stream) const;

        Expression value;

      private:
        static const Token token_value;
    };

    ObjectFile() noexcept;

    void add_constant(std::unique_ptr<Constant> constant);
    void add_function(std::unique_ptr<Function> function);
    void add_macro(std::unique_ptr<Macro> macro);
    void add_object(std::unique_ptr<Object> object) {(void)insert_object(std::move(object));}
    void add_object_file(const std::shared_ptr<ObjectFile>& file);
    bool check_unresolved() const;
    [[nodiscard]] const Constant* constant(Symbol name) const;
    [[nodiscard]] std::vector<Object*> all_objects();
    void collect_explicitly_used_objects(std::unordered_set<Object*>& set) const;
    Object* create_object(Symbol section_name, Visibility visibility, const Token& object_name);
    void evaluate();
    void import(ObjectFile* library);
    [[nodiscard]] const Object* object(Symbol object_name) const;
    void mark_used(Object* object) {explicitly_used_objects.insert(object);}
    void mark_used(Symbol name) {explicitly_used_object_names.insert(name);}
    void pin(Symbol name, Expression address) {pinned_objects[name] = Pinned{name, address};}
    void remove_private_constants();
    void serialize(std::ostream& stream) const;
    void set_target(const Target* new_target);
    std::shared_ptr<Environment> environment(Visibility visibility) const;

    std::shared_ptr<Environment> public_environment;
    std::shared_ptr<Environment> private_environment;

    Symbol name;
    const Target* target = &Target::empty;

    static const unsigned int format_version_major;
    static const unsigned int format_version_minor;

private:
    class Pinned {
    public:
        Pinned(Symbol name, Expression address): name{name}, address{std::move(address)} {}
        Pinned() = default;

        Symbol name;
        Expression address;

        void process(EvaluationContext context);
    };

    Object* insert_object(std::unique_ptr<Object> object);
    void add_to_environment(const Constant& constant) { add_to_environment(constant.name.as_symbol(), constant.visibility, constant.value);}
    void add_to_environment(Object* object);
    void add_to_environment(Symbol symbol_name, Visibility visibility, Expression value) const;

    std::unordered_map<Symbol, std::unique_ptr<Constant>> constants;
    std::unordered_map<Symbol, std::unique_ptr<Function>> functions;
    std::unordered_map<Symbol, std::unique_ptr<Macro>> macros;
    std::unordered_map<Symbol, std::unique_ptr<Object>> objects;

    std::set<Symbol> explicitly_used_object_names;
    std::set<Object*> explicitly_used_objects;
    std::map<Symbol, Pinned> pinned_objects;
    bool have_unresolved{false};

    std::unordered_set<ObjectFile*> imported_libraries;
};

std::ostream& operator<<(std::ostream& stream, const ObjectFile& list);
std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<ObjectFile>& list);

#endif // OBJECT_FILE_H
