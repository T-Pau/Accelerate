/*
AddressingModeMatcher.cc -- 

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

#include "AddressingModeMatcher.h"
#include "ParseException.h"
#include "TokenNode.h"

void AddressingModeMatcher::add_notation(Symbol addressing_mode, size_t notation_index, const AddressingMode::Notation &notation, const std::unordered_map<Symbol, const ArgumentType*>& arguments) {
    start.add_notation(AddressingModeMatcherResult(addressing_mode, notation_index), notation.elements.begin(), notation.elements.end(), arguments);
}

void AddressingModeMatcher::MatcherNode::add_notation(const AddressingModeMatcherResult& result, std::vector<AddressingMode::Notation::Element>::const_iterator current, std::vector<AddressingMode::Notation::Element>::const_iterator end, const std::unordered_map<Symbol, const ArgumentType*>& arguments) {
    if (current == end) {
        results.insert(result);
        return;
    }
    for (const auto& element: AddressingModeMatcherElement::elements_for(*current, arguments)) {
        next[element].add_notation(result, current + 1, end, arguments);
    }
}

std::unordered_set<AddressingModeMatcherResult> AddressingModeMatcher::match(const std::vector<std::shared_ptr<Node>>& nodes) const {
    auto current = nodes.begin();

    const MatcherNode* node = &start;
    while (current != nodes.end()) {
        auto it = node->next.find(AddressingModeMatcherElement(current->get()));
        if (it == node->next.end()) {
            return {};
        }
        current++;
        node = &it->second;
    }

    return node->results;
}


bool AddressingModeMatcherElement::operator==(const AddressingModeMatcherElement &other) const {
    if (type != other.type) {
        return false;
    }

    switch (type) {
        case INTEGER:
            return true;

        case KEYWORD:
        case PUNCTUATION:
            return symbol == other.symbol;
    }
}


std::vector<AddressingModeMatcherElement>
AddressingModeMatcherElement::elements_for(const AddressingMode::Notation::Element &element, const std::unordered_map<Symbol, const ArgumentType*>& arguments) {
    std::vector<AddressingModeMatcherElement> elements;

    switch (element.type) {
        case AddressingMode::Notation::ARGUMENT: {
            auto it = arguments.find(element.symbol);
            if (it == arguments.end()) {
                throw Exception("unknown argument '%s'", element.symbol.c_str());
            }

            auto argument_type = it->second;
            switch (argument_type->type()) {
                case ArgumentType::RANGE:
                case ArgumentType::MAP:
                    elements.emplace_back();
                    break;

                case ArgumentType::ENUM:
                    for (const auto &pair: dynamic_cast<const ArgumentTypeEnum *>(argument_type)->entries) {
                        elements.emplace_back(KEYWORD, pair.first);
                    }
                    break;
            }
            break;
        }

        case AddressingMode::Notation::PUNCTUATION:
            elements.emplace_back(PUNCTUATION, element.symbol);
            break;

        case AddressingMode::Notation::RESERVED_WORD:
            elements.emplace_back(KEYWORD, element.symbol);
            break;
    }
    return elements;
}

AddressingModeMatcherElement::AddressingModeMatcherElement(Node *node) {
    switch (node->type()) {
        case Node::EXPRESSION:
            type = INTEGER;
            break;

        case Node::PUNCTUATION:
            type = PUNCTUATION;
            symbol = dynamic_cast<TokenNode*>(node)->as_symbol();
            break;

        case Node::KEYWORD:
            type = KEYWORD;
            symbol = dynamic_cast<TokenNode*>(node)->as_symbol();
            break;
    }
}
