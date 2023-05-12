/*
BodyBlock.cc --

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

#include "BodyBlock.h"

BodyBlock::BodyBlock(std::vector<Body> elements_): elements(std::move(elements_)) {
    for (const auto&element: elements) {
        size_range_ += element.size_range();
    }
}


std::ostream& operator<<(std::ostream& stream, const BodyBlock& block) {
    block.serialize(stream, "");
    return stream;
}

std::optional<Body> BodyBlock::evaluated(const Environment &environment, const SizeRange& offset) const {
    auto new_elements = std::vector<Body>();
    auto changed = false;
    auto current_offset = offset;

    for (auto& element: elements) {
        auto new_element = element.evaluated(environment, current_offset);
        if (new_element) {
            changed = true;
        }
        else {
            new_element = element;
        }

        if (new_element->empty()) {
            changed = true;
            continue;
        }

        if (!new_elements.empty()) {
            auto combined = new_elements.back().append_sub(*new_element);
            if (combined) {
                changed = true;
                new_elements.pop_back();
                new_element = combined;
            }
        }

        current_offset += new_element->size_range();
        new_elements.emplace_back(*new_element);
    }

    if (changed) {
        return Body(std::make_shared<BodyBlock>(new_elements));
    }
    else {
        return {};
    }
}

void BodyBlock::serialize(std::ostream &stream, const std::string& prefix) const {
    for (auto& element: elements) {
        element.serialize(stream, prefix);
    }
}


std::optional<Body> BodyBlock::append_sub(Body body, Body element) {
    auto block_element = element.as_block();

    auto new_body = body.make_unique();
    auto new_block = new_body.as_block();

    if (block_element) {
        new_block->elements.insert(elements.end(), block_element->elements.begin(), block_element->elements.end());
    }
    else {
        new_block->elements.emplace_back(element);

    }
    new_block->size_range_ += element.size_range();

    return new_body;
}

void BodyBlock::encode(std::string &bytes, const Memory* memory) const {
    for (auto& element: elements) {
        element.encode(bytes, memory);
    }
}
