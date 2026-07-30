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

#include "Body/SimpleRepeatBody.h"

std::optional<Body> SimpleRepeatBody::simplify(Expression start, Expression end, Body body, bool always_create) {
    validate_range(start, end);

    if (auto count_value = count(start, end)) {
        if (*count_value == 1) {
            return body;
        }
    }

    if (always_create) {
        return Body(std::make_shared<SimpleRepeatBody>(start, end, body));
    }
    else {
        return {};
    }
}

void SimpleRepeatBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".repeat " << start << ", " << end << " {" << std::endl;
    body.serialize(stream, prefix + "  ");
    stream << "}" << std::endl;
}

void SimpleRepeatBody::encode(std::string& bytes, const Memory* memory) {
    auto count_value = count();
    if (!count_value) {
        throw LocationException(start.location(), "cannot determine repeat count");
    }
    std::string repeated_bytes;
    body.encode(repeated_bytes, memory);
    for (uint64_t i = 0; i < *count_value; i++) {
        bytes += repeated_bytes;
    }
}

std::optional<Body> SimpleRepeatBody::evaluate_process(const EvaluationContext& context) {
    auto current_count_range = count_range();
    size_range_ = current_count_range * body.size_range();

    return simplify(start, end, body, false);
}
