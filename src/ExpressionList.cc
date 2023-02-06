/*
ExpressionList.cc -- List of Epressions

Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

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

#include "ExpressionList.h"
#include "ParseException.h"

size_t ExpressionList::byte_size() const {
    size_t size = 0;

    for (const auto& expression : expressions) {
        if (expression->byte_size() == 0) {
            return 0;
        }
        size += expression->byte_size();
    }

    return size;
}

void ExpressionList::serialize(std::ostream &stream) const {
    auto first = true;

    for (const auto& expression: expressions) {
        if (first) {
            first = false;
        }
        else {
            stream << ", ";
        }
        stream << expression;
    }
}

std::vector<uint8_t> ExpressionList::bytes(uint64_t byte_order) const {
    std::vector<uint8_t> data;

    for (const auto& expression: expressions) {
        if (!expression->has_value()) {
            throw ParseException(expression->location, "value not known");
        }
        size_t size = expression->byte_size();
        if (size == 0) {
            size = expression->minimum_byte_size();
        }

        Int::encode(data, expression->value(), byte_order);
    }

    return data;
}

std::ostream& operator<<(std::ostream& stream, const ExpressionList& list) {
    list.serialize(stream);
    return stream;
}
