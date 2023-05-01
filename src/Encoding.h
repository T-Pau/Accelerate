/*
Encoding.h -- 

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

#ifndef ENCODING_H
#define ENCODING_H

#include <string>

#include "Expression.h"
#include "Value.h"

class Encoding {
public:
    enum Type {
        SIGNED,
        UNSIGNED
    };

    Encoding(Type type, size_t size, uint64_t byte_order = default_byte_order): type(type), size(size), byte_order(byte_order) {}
    Encoding(const Value value);

    [[nodiscard]] size_t byte_size() const {return size;}
    void encode(std::string& bytes, const Value& value) const;
    [[nodiscard]] bool fits(const Value& value) const;
    [[nodiscard]] bool is_natural_encoding(const Value& value) const;
    void serialize(std::ostream& stream) const;

    std::optional<Value> maximum_value() const;
    std::optional<Value> minimum_value() const;

    bool operator==(const Encoding& other) const;
    bool operator!=(const Encoding& other) const {return !(*this == other);}

    static uint64_t default_byte_order;

private:
    Type type;
    size_t size;
    uint64_t byte_order;
};

std::ostream& operator<<(std::ostream& stream, const Encoding& encoding);

#endif // ENCODING_H
