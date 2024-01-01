/*
RepeatBody.cc -- 

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

#include "RepeatBody.h"

Body RepeatBody::create(Symbol variable, const std::optional<Expression>& start, const Expression& end, const Body& body) {
    if ((!start || start.has_value()) && end.has_value()) {
        auto expanded_body = Body();
        auto step = Value(static_cast<uint64_t>(1));
        auto environment = std::make_shared<Environment>();
        auto result = EvaluationResult();
        auto context = EvaluationContext(result, EvaluationContext::ARGUMENTS, environment);
        for (auto index = start ? *start->value() : Value(static_cast<uint64_t>(0)); index < *end.value(); index += step) {
            if (variable) {
                environment->add(variable, Expression(index));
                auto new_body = body.evaluated(context);
                // TODO: handle result
                expanded_body.append(new_body ? *new_body : body);
            }
            else {
                expanded_body.append(body);
            }
        }
        return expanded_body;
    }
    else {
        return Body(std::make_shared<RepeatBody>(variable, start, end, body));
    }
}

std::optional<Body> RepeatBody::evaluated(const EvaluationContext& context) const {
    auto new_start = start ? start->evaluated(context) : std::optional<Expression>{};
    auto new_end = end.evaluated(context);
    auto new_body = body.evaluated(context);

    if (new_start || new_end || new_body) {
        return create(variable, new_start ? new_start : start, new_end ? *new_end : end, new_body ? *new_body : body);
    }
    else {
        return {};
    }
}

void RepeatBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".repeat ";
    if (variable) {
        stream << variable << ", ";
    }
    if (start) {
        stream << *start << ", ";
    }
    stream << end << " {" << std::endl;
    body.serialize(stream, prefix + "  ");
    stream << "}" << std::endl;
}
