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

std::ostream& operator<<(std::ostream& stream, const BodyBlock& block) {
    block.serialize(stream);
    return stream;
}

BodyElement::EvaluationResult BodyBlock::evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const {
    auto result = EvaluationResult(minimum_offset, maximum_offset);

    auto new_elements = std::vector<std::shared_ptr<BodyElement>>();
    auto changed = false;

    for (auto& element: elements) {
        auto sub_result = element->evaluate(environment, result.minimum_offset, result.maximum_offset);
        if (sub_result.element) {
            changed = true;
            if (!sub_result.element->empty()) {
                new_elements.emplace_back(sub_result.element);
                result.minimum_offset += sub_result.minimum_offset;
                result.maximum_offset += sub_result.maximum_offset;
            }
        }
        else {
            if (element->empty()) {
                changed = true;
            }
            else {
                new_elements.emplace_back(element);
                result.minimum_offset += element->minimum_size();
                result.maximum_offset += element->maximum_size();
            }
        }
    }

    if (changed) {
        result.element = std::make_shared<BodyBlock>(new_elements);
    }

    // minimum_size = result.minimum_offset - minimum_offset;
    // maximum_size = result.maximum_offset - maximum_offset;

    return result;
}

void BodyBlock::serialize(std::ostream &stream, const std::string& prefix) const {
    for (auto& element: elements) {
        element->serialize(stream, prefix);
    }
}

std::optional<uint64_t> BodyBlock::size() const {
    auto total_size = uint64_t(0);

    for (auto& element: elements) {
        auto current_size = element->size();

        if (current_size.has_value()) {
            total_size += *current_size;
        }
        else {
            return {};
        }
    }

    return total_size;
}


uint64_t BodyBlock::maximum_size() const {
    auto total_size = uint64_t(0);

    for (auto& element: elements) {
        total_size += element->maximum_size();
    }

    return total_size;
}


uint64_t BodyBlock::minimum_size() const {
    auto total_size = uint64_t(0);

    for (auto& element: elements) {
        total_size += element->minimum_size();
    }

    return total_size;
}

std::shared_ptr<BodyElement> BodyBlock::append_sub(std::shared_ptr<BodyElement> body, std::shared_ptr<BodyElement> element) {
    auto block_element = std::dynamic_pointer_cast<BodyBlock>(element);

    auto new_block = std::dynamic_pointer_cast<BodyBlock>(BodyElement::make_unique(body));

    if (block_element) {
        new_block->elements.insert(elements.end(), block_element->elements.begin(), block_element->elements.end());
    }
    else {
        new_block->elements.emplace_back(element);
    }

    return new_block;
}

void BodyBlock::encode(std::string &bytes) const {
    for (auto& element: elements) {
        element->encode(bytes);
    }
}