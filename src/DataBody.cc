/*
DataBody.cc --

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

#include "DataBody.h"

#include "Exception.h"

DataBody::DataBody(std::vector<DataBodyElement> data_): data(std::move(data_)) {
    for (const auto& datum: data) {
        size_range_ += datum.size_range();
    }
}


std::optional<Body> DataBody::evaluated(const EvaluationContext& context) const {
    auto new_data = std::vector<DataBodyElement>();
    auto changed = false;

    for (auto& datum: data) {
        auto new_expression = datum.expression.evaluated(context);
        if (new_expression) {
            changed = true;
            new_data.emplace_back(*new_expression, datum.encoding);
        }
        else {
            new_data.emplace_back(datum);
        }
    }

    if (changed) {
        return Body(std::make_shared<DataBody>(new_data));
    }

    return {};
}

void DataBody::serialize(std::ostream &stream, const std::string& prefix) const {
    stream << prefix << ".data ";
    auto first = true;
    for (auto& datum: data) {
        if (first) {
            first = false;
        }
        else {
            stream << ", ";
        }
        stream << datum.expression;
        if (datum.encoding.has_value()) {
            auto value = datum.expression.value();
            if (!value.has_value() || !datum.encoding->is_natural_encoding(*value)) {
                stream << *datum.encoding;
            }
        }
    }

    stream << std::endl;
}

std::optional<Body> DataBody::append_sub(Body body, Body element) {
    auto data_element = element.as_data();

    if (!data_element) {
        return {};
    }

    auto new_body = body.make_unique();
    auto new_data_body = new_body.as_data();

    new_data_body->data.insert(new_data_body->data.end(), data_element->data.begin(), data_element->data.end());
    new_data_body->size_range_ += element.size_range();
    return new_body;
}

void DataBody::encode(std::string &bytes, const Memory* memory) const {
    for (auto& datum: data) {
        auto value = datum.expression.value();
        if (!value.has_value()) {
            throw Exception("unknown value");
        }

        if (datum.encoding.has_value()) {
            datum.encoding->encode(bytes, *value);
        }
        else {
            Encoding(*value).encode(bytes, *value);
        }
    }
}

void DataBody::collect_objects(std::unordered_set<Object *> &objects) const {
    for (auto& datum: data) {
        datum.expression.collect_objects(objects);
    }
}

Body DataBody::appending(const std::vector<DataBodyElement> &elements) {
    auto new_data = data;
    new_data.insert(new_data.end(), elements.begin(), elements.end());
    return Body(new_data);
}

SizeRange DataBodyElement::size_range() const {
    if (encoding.has_value()) {
        return SizeRange(encoding->byte_size());
    }
    auto value = expression.value();
    if (value) {
        return SizeRange(value->default_size());
    }
    auto minimum_value = expression.minimum_value();
    auto maximum_value = expression.maximum_value();
    if (minimum_value && maximum_value) {
        auto minimum_size = minimum_value->default_size();
        auto maximum_size = maximum_value->default_size();
        return {std::min(minimum_size, maximum_size), std::max(minimum_size, maximum_size)};
    }
    return {1, 8};
}
