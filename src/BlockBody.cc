/*
BlockBody.cc --

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

#include "BlockBody.h"

#include "DataBody.h"

BlockBody::BlockBody(std::vector<Body> elements_): elements(std::move(elements_)) {
    for (const auto& element: elements) {
        size_range_ += element.size_range();
    }
}


Body BlockBody::create(const std::vector<Body>& elements) {
    auto block = std::make_shared<BlockBody>();

    for (auto& element: elements) {
        block->append_element(element);
    }

    switch (block->elements.size()) {
        case 0:
            return {};

        case 1:
            return block->elements.front();

        default:
            return Body(block);
    }
}


std::ostream& operator<<(std::ostream& stream, const BlockBody& block) {
    block.serialize(stream, "");
    return stream;
}

std::optional<Body> BlockBody::evaluated(const Environment &environment, const SizeRange& offset) const {
    auto new_elements = std::vector<Body>();
    auto changed = false;
    auto current_offset = offset;

    for (auto& element: elements) {
        auto new_element = element.evaluated(environment, current_offset);
        if (new_element) {
            changed = true;
            new_elements.emplace_back(*new_element);
        }
        else {
            new_elements.emplace_back(element);
        }

        current_offset += new_elements.back().size_range();
    }

    if (changed) {
        return BlockBody::create(new_elements);
    }
    else {
        return {};
    }
}

void BlockBody::serialize(std::ostream &stream, const std::string& prefix) const {
    for (auto& element: elements) {
        element.serialize(stream, prefix);
    }
}


std::optional<Body> BlockBody::append_sub(Body body, Body element) {
    auto new_body = body.make_unique();

    new_body.as_block()->append_element(element);

    return new_body;
}

void BlockBody::encode(std::string &bytes, const Memory* memory) const {
    for (auto& element: elements) {
        element.encode(bytes, memory);
    }
}

void BlockBody::append_element(const Body &element) {
    if (element.empty()) {
        return;
    }
    if (element.is_block()) {
        auto block = element.as_block();
        if (!elements.empty() && elements.back().is_data() && block->elements.front().is_data()) {
            auto last_data = elements.back().as_data();
            auto first_data = block->elements.front().as_data();
            elements.pop_back();
            elements.emplace_back(last_data->appending(first_data));
            elements.insert(elements.end(), block->elements.begin() + 1, block->elements.end());
        }
        else {
            elements.insert(elements.end(), block->elements.begin(), block->elements.end());
        }
    }
    else if (element.is_data() && !elements.empty() && elements.back().is_data()) {
        auto last_data = elements.back().as_data();
        auto new_data = element.as_data();
        elements.pop_back();
        elements.emplace_back(last_data->appending(new_data));
    }
    else {
        elements.emplace_back(element);
    }
}