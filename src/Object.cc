/*
Object.cc --

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

#include "Object.h"

#include "BodyElement.h"
#include "ObjectFile.h"

Object::Object(const ObjectFile *owner, const MemoryMap::Section *section, Object::Visibility visibility, Token name): owner(owner), section(section), visibility(visibility), name(name), environment(std::make_shared<Environment>(owner->local_environment)) {}

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Object>& object) {
    object->serialize(stream);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const Object& object) {
    object.serialize(stream);
    return stream;
}

std::ostream& operator<< (std::ostream& stream, Object::Visibility visibility) {
    switch  (visibility) {
        case Object::OBJECT:
            stream << "none";
            break;
        case Object::LOCAL:
            stream << "local";
            break;
        case Object::GLOBAL:
            stream << "global";
            break;
    }
    return stream;
}

void Object::serialize(std::ostream &stream) const {
    stream << ".object " << name.as_string() << " {" << std::endl;
    if (alignment > 0) {
        stream << "    alignment: " << alignment << std::endl;
    }
    stream << "    section: " << section->name << std::endl;
    stream << "    visibility: " << visibility << std::endl;
    if (!body.empty()) {
        stream << "    body <" << std::endl;
        body.serialize(stream, "        ");
        stream << "    >" << std::endl;
    }
    else {
        stream << "    reserve: " << reservation << std::endl;
    }

    stream << "}" << std::endl;
}

Object::Object(const ObjectFile *owner, const Object *object): owner(owner), section(object->section), visibility(object->visibility), name(object->name), reservation(object->reservation) {
    // TODO: copy constructor would need cloning data. move constructor instead?
    if (!object->is_reservation()) {
        // append(object->data);
    }
}

void Object::evaluate(std::shared_ptr<Environment> environment) {
    body.evaluate(name.as_symbol(), environment);
}

SizeRange Object::size_range() const {
    if (is_reservation()) {
        return SizeRange(reservation);
    }
    else {
        return body.size_range();
    }
}
