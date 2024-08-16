/*
FillExpression.cc --

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

#include "FillExpression.h"


#include "Encoder.h"
#include "ParseException.h"

Expression FillExpression::create(const Location& location, const std::vector<Expression>& arguments) {
    if (arguments.size() != 2) {
        throw ParseException(location, "invalid number of arguments");
    }
    return create(location, arguments[0], arguments[1]);
}

Expression FillExpression::create(const Location& location, const Expression& count, const Expression& value) {
    if (count.has_value() && value.has_value()) {
        const auto real_count = *count.value();
        if (!real_count.is_unsigned()) {
            throw ParseException(count.location(), ".fill count must be unsigned");
        }
        auto actual_count = real_count.unsigned_value();
        const auto real_value = *value.value();
        auto encoding = Encoder(real_value);
        auto bytes = std::string{};
        encoding.encode(bytes, real_value);
        auto result = std::string{};
        result.reserve(actual_count * bytes.size());
        for (uint64_t i = 0; i < actual_count; i++) {
            result += bytes;
        }
        return Expression(location, Value(result));
    }
    else {
        return Expression(std::make_shared<FillExpression>(location, count, value));
    }
}

std::optional<Expression> FillExpression::evaluated(const EvaluationContext& context) const {
    auto new_count = count.evaluated(context);
    auto new_value = value.evaluated(context);
    if (new_count || new_value) {
        return create(location, new_count.value_or(count), new_value.value_or(value));
    }
    else {
        return {};
    }
}

void FillExpression::serialize_sub(std::ostream& stream) const {
    stream << ".fill(" << count << ", " << value << ")";
}
