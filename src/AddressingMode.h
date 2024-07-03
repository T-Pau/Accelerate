/*
AddressingMode.h -- 

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

#ifndef ADDRESSING_MODE_H
#define ADDRESSING_MODE_H

#include <utility>

#include "ArgumentType.h"
#include "DataBody.h"
#include "BaseExpression.h"
#include "ParsedValue.h"
#include "Symbol.h"
#include "Token.h"

class AddressingMode {
public:
    class Notation {
    public:
        enum Type {
            ARGUMENT,
            PUNCTUATION,
            RESERVED_WORD
        };
        class Element {
        public:
            Element(Type type, Symbol symbol): type(type), symbol(symbol) {}

            [[nodiscard]] bool is_argument() const {return type == ARGUMENT;}

            Type type;
            Symbol symbol;
        };

        std::vector<Element> elements;
    };

    class Argument {
      public:
        explicit Argument(const ArgumentType* type, std::optional<Value> default_value = {}) : type{type}, default_value{std::move(default_value)} {}
        Argument() = default;

        const ArgumentType* type{};
        std::optional<Value> default_value;
    };

    [[nodiscard]] const Argument* argument(Symbol name) const;
    [[nodiscard]] bool has_argument(Symbol name) const {return argument(name) != nullptr;}

    bool operator<(const AddressingMode& other) const {return priority < other.priority;}

    size_t priority = std::numeric_limits<size_t>::max();
    bool uses_pc = false;
    std::vector<Notation> notations;
    std::unordered_map<Symbol, std::unique_ptr<Argument>> arguments;
    Body encoding;

    void add_notation(Notation notation) {notations.emplace_back(std::move(notation));}
};

#endif // ADDRESSING_MODE_H
