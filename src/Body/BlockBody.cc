/*
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

BlockBody::BlockBody(std::vector<Body> block_) : block(std::move(block_)) {
    for (const auto& element : block) {
        size_range_ += element.size_range();
    }
}

Body BlockBody::create(const std::vector<Body>& elements) {
    auto block = std::make_shared<BlockBody>();

    for (auto& element : elements) {
        block->append_element(element);
    }

    switch (block->block.size()) {
        case 0:
            return {};

        case 1:
            return block->block.front();

        default:
            return Body(block);
    }
}

std::ostream& operator<<(std::ostream& stream, const BlockBody& block) {
    block.serialize(stream, "");
    return stream;
}

std::optional<Body> BlockBody::evaluate(const EvaluationContext& context) {
    auto current_offset = context.offset;
    auto new_size_range = SizeRange(0, 0);
    auto new_block = std::vector<Body>();

    std::optional<Body> previous_element;

    for (auto& element : block) {
        element.evaluate(context.setting_offset(current_offset));
        current_offset += element.size_range();
        new_size_range += element.size_range();
        if (!element.empty()) {
            if (previous_element) {
                auto [success, new_body] = previous_element->append_sub(element);
                if (success) {
                    if (new_body) {
                        new_block.back() = *new_body;
                    }
                    continue;
                }
            }
            new_block.emplace_back(element);
            previous_element = element;
        }
    }

    block = std::move(new_block);

    if (block.empty()) {
        return Body();
    }
    else if (block.size() == 1) {
        return block.front();
    }
    else {
        size_range_ = new_size_range;
        return {};
    }
}

void BlockBody::serialize(std::ostream& stream, const std::string& prefix) const {
    for (auto& element : block) {
        element.serialize(stream, prefix);
    }
}

std::pair<bool, std::optional<Body>> BlockBody::append_sub(const Body& body, const Body& element) {
    append_element(element);
    return {true, {}};
}

void BlockBody::encode(std::string& bytes, const Memory* memory) const {
    for (auto& element : block) {
        element.encode(bytes, memory);
    }
}

void BlockBody::append_element(const Body& element) {
    if (element.empty()) {
        return;
    }
    if (auto new_block = element.as<BlockBody>()) {
        if (!block.empty() && block.back().is<DataBody>() && new_block->block.front().is<DataBody>()) {
            auto last_data = block.back().as<DataBody>();
            auto first_data = new_block->block.front().as<DataBody>();
            block.pop_back();
            block.emplace_back(last_data->appending(first_data));
            block.insert(block.end(), new_block->block.begin() + 1, new_block->block.end());
        }
        else {
            block.insert(block.end(), new_block->block.begin(), new_block->block.end());
        }
    }
    else if (element.is<DataBody>() && !block.empty() && block.back().is<DataBody>()) {
        auto last = block.back(); // needed to keep alive after we pop it off block.
        auto last_data = block.back().as<DataBody>();
        auto new_data = element.as<DataBody>();
        block.pop_back();
        block.emplace_back(last_data->appending(new_data));
    }
    else {
        block.emplace_back(element);
    }
    size_range_ += element.size_range();
}

void BlockBody::traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    for (auto& element : block) {
        body_callable(element);
    }
}
