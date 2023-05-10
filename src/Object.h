/*
Object.h --

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#ifndef OBJECT_H
#define OBJECT_H

#include <vector>

#include "Body.h"
#include "MemoryMap.h"
#include "Token.h"

class ObjectFile;

class Object {
public:
    enum Visibility {
        OBJECT,
        LOCAL,
        GLOBAL
    };

    Object(const ObjectFile* owner, const MemoryMap::Section* section, Visibility visibility, Token name): owner(owner), section(section), visibility(visibility), name(name) {}
    Object(const ObjectFile* owner, const Object* object);

    void evaluate(const Environment& environment);
    [[nodiscard]] bool is_reservation() const {return body.empty();}
    [[nodiscard]] bool empty() const {return is_reservation() && size == 0;}
    [[nodiscard]] bool has_address() const {return address.has_value();}

    void serialize(std::ostream& stream) const;

    const ObjectFile* owner;
    const MemoryMap::Section* section;
    Visibility visibility;
    Token name;
    uint64_t alignment = 0;
    uint64_t size = 0;
    std::optional<uint64_t> bank;
    std::optional<uint64_t> address;

    Body body;
};

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Object>& node);
std::ostream& operator<<(std::ostream& stream, const Object& node);
std::ostream& operator<<(std::ostream& stream, Object::Visibility visibility);


#endif // OBJECT_H
