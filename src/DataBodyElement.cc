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

std::optional<uint64_t> DataBodyElement::size() const {
    size_t size = 0;

    for (const auto& datum: data) {
        auto datum_size = datum.size();
        if (!datum_size.has_value()) {
            return {};
        }
        size += *datum_size;
    }

    return size;
}

uint64_t DataBodyElement::maximum_size() const {
    size_t size = 0;

    for (const auto& datum: data) {
        size += datum.maximum_size();
    }

    return size;
}

uint64_t DataBodyElement::minimum_size() const {
    size_t size = 0;

    for (const auto& datum: data) {
        size += datum.minimum_size();
    }

    return size;
}

BodyElement::EvaluationResult DataBodyElement::evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const {
    auto result = EvaluationResult(minimum_offset, maximum_offset);

    auto new_data = std::vector<Datum>();
    auto changed = false;

    for (auto& datum: data) {
        auto new_expression = datum.expression->evaluate(environment);
        if (new_expression) {
            changed = true;
            new_data.emplace_back(new_expression, datum.encoding);
        }
        else {
            new_data.emplace_back(datum);
        }
        result.minimum_offset += new_data.back().minimum_size();
        result.maximum_offset += new_data.back().maximum_size();
    }

    if (changed) {
        result.element = std::make_shared<DataBodyElement>(new_data);
    }

    // minimum_size = result.minimum_offset - minimum_offset;
    // maximum_size = result.maximum_offset - maximum_offset;

    return result;
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
            auto value = datum.expression->value();
            if (!value.has_value() || !datum.encoding->is_natural_encoding(*value)) {
                stream << *datum.encoding;
            }
        }
    }

    stream << std::endl;
}

std::shared_ptr<BodyElement>
DataBodyElement::append_sub(std::shared_ptr<BodyElement> body, std::shared_ptr<BodyElement> element) {
    auto data_element = std::dynamic_pointer_cast<DataBodyElement>(element);

    if (!data_element) {
        return {};
    }

    auto new_data_body = std::dynamic_pointer_cast<DataBodyElement>(BodyElement::make_unique(body));

    new_data_body->data.insert(new_data_body->data.end(), data_element->data.begin(), data_element->data.end());

    return new_data_body;
}

void DataBodyElement::encode(std::string &bytes) const {
    for (auto& datum: data) {
        auto value = datum.expression->value();
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

std::optional<uint64_t> DataBodyElement::Datum::size() const {
    if (encoding.has_value()) {
        return encoding->byte_size();
    }
    else {
        const auto& value = expression->value();
        if (value.has_value()) {
            return value->default_size();
        }
        else {
            return {};
        }
    }
}
