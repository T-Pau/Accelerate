/*
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

#include "LabelOffsetExpression.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "Body/LabelBody.h"
#include "Expression/ValueExpression.h"

using namespace tpau::cpp_kernal;

LabelOffsetExpression::LabelOffsetExpression(const Location& location, Symbol entity_name, Symbol label_name, size_t unnamed_label_index, std::shared_ptr<LabelBody> body) : BaseExpression(location), entity_name(entity_name), label_name(label_name), unnamed_label_index(unnamed_label_index), body_(body) {
    if (!body) {
        throw LocationException(location, "internal error: LabelOffsetExpression: body is null");
    }
    body->add_reference();
}

LabelOffsetExpression::~LabelOffsetExpression() {
    if (auto label_body = body_.lock()) {
        label_body->remove_reference();
    }
}

std::optional<Expression> LabelOffsetExpression::evaluate(const EvaluationContext& context) {
    // TODO: make sure body is evaluated first?
    if (has_value()) {
        return ValueExpression::create(location, *value());
    }
    else {
        return {};
    }
}

void LabelOffsetExpression::serialize_sub(std::ostream& stream) const {
    stream << "label_offset(" << entity_name << ",";
    if (!label_name.empty()) {
        stream << label_name;
    }
    else {
        stream << unnamed_label_index;
    }
    stream << ")";
}

std::shared_ptr<LabelBody> LabelOffsetExpression::body() const {
    if (auto label_body = body_.lock()) {
        return label_body;
    }
    else {
        throw LocationException(location, "internal error: LabelOffsetExpression body disappeared");
    }
}
