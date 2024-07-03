/*
AddressingModeMatcher.h -- 

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

#ifndef ADDRESSING_MODE_MATCHER_H
#define ADDRESSING_MODE_MATCHER_H

#include <unordered_map>
#include <unordered_set>

#include "Node.h"
#include "Symbol.h"
#include "AddressingMode.h"

class AddressingModeMatcherElement {
public:
    enum Type {
        PUNCTUATION,
        KEYWORD,
        INTEGER
    };

    AddressingModeMatcherElement(Type type, Symbol symbol): type(type), symbol(symbol) {}
    AddressingModeMatcherElement(): type(INTEGER) {}
    explicit AddressingModeMatcherElement(Node* node);

    bool operator==(const AddressingModeMatcherElement& other) const;

    Type type = INTEGER;
    Symbol symbol;

    static std::vector<AddressingModeMatcherElement> elements_for(const AddressingMode::Notation::Element& element, const std::unordered_map<Symbol, std::unique_ptr<AddressingMode::Argument>>& arguments);
};


template<>
struct std::hash<AddressingModeMatcherElement>
{
    std::size_t operator()(AddressingModeMatcherElement const& element) const noexcept {
        size_t h1 = std::hash<size_t>{}(static_cast<size_t>(element.type));
        size_t h2 = 0;
        switch (element.type) {
            case AddressingModeMatcherElement::INTEGER:
                break;

            case AddressingModeMatcherElement::KEYWORD:
            case AddressingModeMatcherElement::PUNCTUATION:
                h2 = std::hash<Symbol>{}(element.symbol);
                break;
        }
        return h1 ^ (h2 << 1);
    }
};


class AddressingModeMatcherResult {
public:
    AddressingModeMatcherResult(Symbol addressing_mode, size_t notation_index): addressing_mode(addressing_mode), notation_index(notation_index) {}
    bool operator==(const AddressingModeMatcherResult& other) const {return addressing_mode == other.addressing_mode && notation_index == other.notation_index;}

    Symbol addressing_mode;
    size_t notation_index;
};


template<>
struct std::hash<AddressingModeMatcherResult>
{
    std::size_t operator()(AddressingModeMatcherResult const& result) const noexcept {
        return std::hash<Symbol>{}(result.addressing_mode) ^ (std::hash<size_t>{}(result.notation_index) << 1);
    }
};


class AddressingModeMatcher {
public:
    [[nodiscard]] std::unordered_set<AddressingModeMatcherResult> match(const std::vector<std::shared_ptr<Node>>& nodes) const;

    void add_notation(Symbol addressing_mode, size_t notation_index, const AddressingMode::Notation& notation, const std::unordered_map<Symbol, std::unique_ptr<AddressingMode::Argument>>& arguments);

private:
    class MatcherNode {
    public:
        void add_notation(const AddressingModeMatcherResult& result, std::vector<AddressingMode::Notation::Element>::const_iterator current, std::vector<AddressingMode::Notation::Element>::const_iterator end, const std::unordered_map<Symbol, std::unique_ptr<AddressingMode::Argument>>& arguments);

        std::unordered_set<AddressingModeMatcherResult> results;
        std::unordered_map<AddressingModeMatcherElement, std::unique_ptr<MatcherNode>> next;
    };

    MatcherNode start;
};


#endif // ADDRESSING_MODE_MATCHER_H
