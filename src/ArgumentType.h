/*
ArgumentType.h -- 

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

#ifndef ARGUMENT_TYPE_H
#define ARGUMENT_TYPE_H

#include <unordered_map>

#include "BaseExpression.h"
#include "Int.h"
#include "IntegerEncoding.h"
#include "Symbol.h"
#include "Value.h"

class ArgumentTypeEncoding;

class ArgumentType {
public:
    enum Type {
        ANY,
        ENCODING,
        ENUM,
        MAP,
        RANGE
    };

    explicit ArgumentType(Symbol name): name(name) {}

    virtual ~ArgumentType() = default;

    [[nodiscard]] virtual Type type() const = 0;

    [[nodiscard]] bool is_encoding() const {return type() == ENCODING;}
    [[nodiscard]] const ArgumentTypeEncoding* as_encoding() const;

    Symbol name;
};

class ArgumentTypeAny: public ArgumentType {
public:
    explicit ArgumentTypeAny(Symbol name): ArgumentType(name) {}

    [[nodiscard]] Type type() const override {return ANY;}
};

class ArgumentTypeEncoding: public ArgumentType {
public:
    explicit ArgumentTypeEncoding(Symbol name, IntegerEncoding encoding): ArgumentType(name), encoding(encoding) {}
    [[nodiscard]] Type type() const override {return ENCODING;}

    [[nodiscard]] std::unique_ptr<ArgumentType> range_type(Symbol range_name) const;

    IntegerEncoding encoding;
};

class ArgumentTypeEnum: public ArgumentType {
public:
    explicit ArgumentTypeEnum(Symbol name): ArgumentType(name) {}

    [[nodiscard]] Type type() const override {return ENUM;}

    [[nodiscard]] bool has_entry(Symbol name) const {return entries.find(name) != entries.end();}
    [[nodiscard]] Value entry(Symbol name) const;

    std::unordered_map<Symbol, Value> entries;
};




class ArgumentTypeMap: public ArgumentType {
public:
    explicit ArgumentTypeMap(Symbol name): ArgumentType(name) {}

    [[nodiscard]] Type type() const override {return MAP;}

    [[nodiscard]] bool has_entry(Value value) const {return entries.find(value) != entries.end();}
    [[nodiscard]] Value entry(Value value) const;

    std::unordered_map<Value, Value> entries;
};




class ArgumentTypeRange: public ArgumentType {
public:
    explicit ArgumentTypeRange(Symbol name): ArgumentType(name) {}

    [[nodiscard]] Type type() const override {return RANGE;}

    Value lower_bound;
    Value upper_bound;
};

#endif // ARGUMENT_TYPE_H
