/*
DataBodyElement.cc --

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

#include "DataBodyElement.h"

#include "Exception.h"

DataBodyElement::DataBodyElement(std::vector<Datum> data_): data(std::move(data_)) {
    for (const auto& datum: data) {
        size_range_ += datum.size_range();
    }
}


std::optional<Body> DataBodyElement::evaluated(const Environment &environment, const SizeRange& offset) const {
    auto new_data = std::vector<Datum>();
    auto changed = false;

    for (auto& datum: data) {
        auto new_expression = datum.expression.evaluated(environment);
        if (new_expression) {
            changed = true;
            new_data.emplace_back(*new_expression, datum.encoding);
        }
        else {
            new_data.emplace_back(datum);
        }
    }

    if (changed) {
        return Body(std::make_shared<DataBodyElement>(new_data));
    }

    return {};
}

void DataBodyElement::serialize(std::ostream &stream, const std::string& prefix) const {
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

std::optional<Body> DataBodyElement::append_sub(Body body, Body element) {
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

void DataBodyElement::encode(std::string &bytes, const Memory* memory) const {
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

void DataBodyElement::collect_objects(std::unordered_set<Object *> &objects) const {
    for (auto& datum: data) {
        datum.expression.collect_objects(objects);
    }
}

std::optional<uint64_t> DataBodyElement::Datum::size() const {
    if (encoding.has_value()) {
        return encoding->byte_size();
    }
    else {
        const auto& value = expression.value();
        if (value.has_value()) {
            return value->default_size();
        }
        else {
            return {};
        }
    }
}
