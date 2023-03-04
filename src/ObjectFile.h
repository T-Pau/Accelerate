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

#include "SymbolTable.h"
#include "Object.h"

class ObjectFile {
public:
    class Constant {
    public:
        Constant() = default;
        Constant(symbol_t name, Object::Visibility visibility, std::shared_ptr<Expression> value): name(name), visibility(visibility), value(std::move(value)) {}

        void serialize(std::ostream& stream) const;

        symbol_t name = 0;
        Object::Visibility visibility = Object::NONE;
        std::shared_ptr<Expression> value;
    };

    void add_constant(symbol_t name, Object::Visibility visibility, std::shared_ptr<Expression> value);
    void add_object(symbol_t name, std::shared_ptr<Object> object) { objects[name] = std::move(object);} // TODO: check for duplicates
    void add_object_file(const ObjectFile& file);

    [[nodiscard]] std::shared_ptr<Object> object(symbol_t name) const;

    void evaluate(const Environment& environment);
    void export_constants(Environment& environment) const;
    void remove_local_constants();

    void serialize(std::ostream& stream) const;

    std::map<symbol_t, std::shared_ptr<Object>> objects;
    std::map<symbol_t, Constant> constants;
};

std::ostream& operator<<(std::ostream& stream, const ObjectFile& list);

#endif // OBJECT_FILE_H
