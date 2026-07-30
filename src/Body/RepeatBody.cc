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

#include "Body/RepeatBody.h"

void RepeatBody::validate_range(const Expression& start, const Expression& end) {
    if (!start.has_type(Value::INTEGER).value_or(true)) {
        throw LocationException(start.location(), "start must be an integer");
    }
    if (!end.has_type(Value::INTEGER).value_or(true)) {
        throw LocationException(end.location(), "end must be an integer");
    }
    if (start.has_value() && end.has_value()) {
        if (*start.value() >= *end.value()) {
            throw LocationException(start.location(), "start must be less than end");
        }
    }
}

std::optional<uint64_t> RepeatBody::count(const Expression& start, const Expression& end) {
    try {
        if (auto count_value = (end.value() - start.value())) {
            if (count_value->is_signed() || count_value->unsigned_value() == 0) {
                throw LocationException(start.location(), "start must be less than end");
            }
            return count_value->unsigned_value();
        }
        else {
            return {};
        }
    }
    catch (const Exception& ex) {
        throw LocationException(start.location(), "invalid repeat count: {}", ex.what());
    }
}

SizeRange RepeatBody::count_range() const {
    auto minimum_start = start.minimum_value();
    auto maximum_start = start.maximum_value();
    auto minimum_end = end.minimum_value();
    auto maximum_end = end.maximum_value();

    auto minimum_count = uint64_t{1};
    auto maximum_count = std::optional<uint64_t>{};

    // end - start
    if (maximum_end && minimum_start) {
        auto maximum_count_value = *maximum_end - *minimum_start;
        if (maximum_count_value.is_signed() || maximum_count_value.unsigned_value() == 0) {
            throw LocationException(start.location(), "start must be less than end");
        }
        maximum_count = maximum_count_value.unsigned_value();
    }

    if (minimum_end && maximum_start) {
        auto minimum_count_value = *minimum_end - *maximum_start;
        if (minimum_count_value.is_signed() || minimum_count_value.unsigned_value() == 0) {
            throw LocationException(start.location(), "start must be less than end");
        }
        minimum_count = minimum_count_value.unsigned_value();
    }
    return SizeRange{minimum_count, maximum_count};
}

void RepeatBody::traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    expression_callable(start);
    expression_callable(end);
    body_callable(body);
}
