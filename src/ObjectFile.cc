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
    for (const auto& pair : constants) {
        stream << pair.second;
    }

    for (const auto& pair: objects) {
        stream << pair.second;
    }
}

void ObjectFile::add_constant(symbol_t name, Object::Visibility visibility, std::shared_ptr<ExpressionNode> value) {
    // TODO: check for duplicates;
    constants[name] = Constant(name, visibility, std::move(value));
}

void ObjectFile::Constant::serialize(std::ostream &stream) const {
    stream << ".constant " << SymbolTable::global[name] << " {" << std::endl;
    // TODO: visibility
    stream << "    value: " << value << std::endl;
    stream << "}" << std::endl;
}
