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
#include <utility>

#include "Symbol.h"
#include "Object.h"
#include "Target.h"

class ObjectFile {
public:
    class Constant {
    public:
        Constant() = default;
        Constant(Symbol name, Object::Visibility visibility, std::shared_ptr<Expression> value): name(name), visibility(visibility), value(std::move(value)) {}

        void serialize(std::ostream& stream) const;

        Symbol name;
        Object::Visibility visibility = Object::OBJECT;
        std::shared_ptr<Expression> value;
    };

    ObjectFile() noexcept;

    Object* create_object(Symbol section_name, Object::Visibility visibility, Token name);

    void add_constant(Symbol name, Object::Visibility visibility, std::shared_ptr<Expression> value);
    void add_object(const Object* object);
    void add_object_file(const ObjectFile& file);

    [[nodiscard]] const Object* object(Symbol name) const;
    [[nodiscard]] std::vector<Object*> all_objects();

    std::shared_ptr<Environment> global_environment;
    std::shared_ptr<Environment> local_environment;

    void evaluate(const Environment& environment);
    void export_constants(Environment& environment) const;
    void remove_local_constants();

    void serialize(std::ostream& stream) const;

    const Target* target = &Target::empty;

    static const unsigned int format_version_major;
    static const unsigned int format_version_minor;

private:
    Object* insert_object(Object object);
    void add_to_environment(const Constant& constant) { add_to_environment(constant.name, constant.visibility, constant.value);}
    void add_to_environment(Object* object);
    void add_to_environment(Symbol name, Object::Visibility visibility, std::shared_ptr<Expression> value);

    std::unordered_map<Symbol, Object> objects;
    std::unordered_map<Symbol, Constant> constants;
};

std::ostream& operator<<(std::ostream& stream, const ObjectFile& list);

#endif // OBJECT_FILE_H
