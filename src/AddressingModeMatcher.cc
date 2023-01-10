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

void AddressingModeMatcher::add_notation(symbol_t addressing_mode, const AddressingMode::Notation &notation) {
    auto node = &start;

    for (const auto& notation_element: notation.elements) {
        node = &node->next[AddressingModeMatcherElement(notation_element)];
    }

    node->addressing_modes.insert(addressing_mode);
}

std::unordered_set<symbol_t> AddressingModeMatcher::match(const std::vector<std::shared_ptr<Node>>& nodes) {
    // TODO: implement
    return {};
}


bool AddressingModeMatcherElement::operator==(const AddressingModeMatcherElement &other) const {
    if (type != other.type) {
        return false;
    }

    switch (type) {
        case INTEGER:
            return true;

        case KEYWORD:
            return value.symbol == other.value.symbol;

        case PUNCTUATION:
            return value.token_type == other.value.token_type;
    }
}


AddressingModeMatcherElement::AddressingModeMatcherElement(AddressingMode::Notation::Element notation_element): value() {
    switch (notation_element.type) {
        case AddressingMode::Notation::ARGUMENT:
            type = INTEGER;
            break;

        case AddressingMode::Notation::PUNCTUATION:
            type = PUNCTUATION;
            value.token_type = notation_element.value.token_type;
            break;

        case AddressingMode::Notation::RESERVED_WORD:
            type = KEYWORD;
            value.symbol = notation_element.value.symbol;
            break;
    }
}

AddressingModeMatcherElement::AddressingModeMatcherElement(Node *node): type(INTEGER), value() {
    // TODO: implement
}
