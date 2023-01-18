/*
ArgumentType.h -- 

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

#ifndef ARGUMENT_TYPE_H
#define ARGUMENT_TYPE_H

#include <unordered_map>

#include "SymbolTable.h"

class ArgumentType {
public:
    enum Type {
        RANGE,
        ENUM,
        MAP
    };

    virtual ~ArgumentType() = default;

    [[nodiscard]] virtual Type type() const = 0;
};


class ArgumentTypeEnum: public ArgumentType {
public:
    [[nodiscard]] Type type() const override {return ENUM;}

    [[nodiscard]] bool has_entry(symbol_t name) const {return entries.find(name) != entries.end();}
    [[nodiscard]] uint64_t entry(symbol_t name) const;

    std::unordered_map<symbol_t, uint64_t> entries;
};




class ArgumentTypeMap: public ArgumentType {
public:
    [[nodiscard]] Type type() const override {return MAP;}

    std::unordered_map<uint64_t, uint64_t> entries;
};




class ArgumentTypeRange: public ArgumentType {
public:
    [[nodiscard]] Type type() const override {return RANGE;}

    int64_t lower_bound;
    uint64_t upper_bound;
};

#endif // ARGUMENT_TYPE_H
