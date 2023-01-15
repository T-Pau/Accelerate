/*
AddressingModeMatcher.cc -- 

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

#include "AddressingModeMatcher.h"
#include "ParseException.h"
#include "TokenNode.h"

void AddressingModeMatcher::add_notation(symbol_t addressing_mode, const AddressingMode::Notation &notation) {
    auto node = &start;

    for (const auto& notation_element: notation.elements) {
        node = &node->next[AddressingModeMatcherElement(notation_element)];
    }

    node->addressing_modes.insert(addressing_mode);
}

std::unordered_set<symbol_t> AddressingModeMatcher::match(const std::vector<std::shared_ptr<Node>>& nodes) const {
    auto current = nodes.begin();

    const MatcherNode* node = &start;
    while (current != nodes.end()) {
        auto it = node->next.find(AddressingModeMatcherElement(current->get()));
        if (it == node->next.end()) {
            return {};
        }
        node = &it->second;
    }

    return node->addressing_modes;
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


AddressingModeMatcherElement::AddressingModeMatcherElement(AddressingMode::Notation::Element notation_element) {
    switch (notation_element.type) {
        case AddressingMode::Notation::ARGUMENT:
            type = INTEGER;
            break;

        case AddressingMode::Notation::PUNCTUATION:
            type = PUNCTUATION;
            symbol = notation_element.symbol;
            break;

        case AddressingMode::Notation::RESERVED_WORD:
            type = KEYWORD;
            symbol = notation_element.symbol;
            break;
    }
}

AddressingModeMatcherElement::AddressingModeMatcherElement(Node *node) {
    switch (node->type()) {
        case Node::INTEGER:
        case Node::EXPRESSION:
        case Node::VARIABLE:
            type = INTEGER;
            break;

        case Node::PUNCTUATION:
            type = PUNCTUATION;
            symbol = dynamic_cast<TokenNode*>(node)->as_symbol();
            break;

        case Node::INSTRUCTION:
            throw ParseException(node->location, "instruction not allowed as argument");
            break;
        case Node::LABEL:
            throw ParseException(node->location, "label not allowed as argument");
            break;

        case Node::KEYWORD:
            type = KEYWORD;
            symbol = dynamic_cast<TokenNode*>(node)->as_symbol();
            break;
    }
    // TODO: implement
}
