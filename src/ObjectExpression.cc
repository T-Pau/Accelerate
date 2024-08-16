/*
ObjectExpression.cc --

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

#include "ObjectExpression.h"

#include "ValueExpression.h"

std::optional<Expression> ObjectExpression::evaluated(const EvaluationContext& context) const {
    if (object->has_address()) {
        return Expression(location, object->address->address);
    }
    else {
        return {};
    }
}

std::optional<Value> ObjectExpression::value() const {
    if (has_value()) {
        return Value(object->address->address);
    }
    else {
        return {};
    }
}

Expression ObjectExpression::create(const Location& location, Object *object) {
    if (object->has_address()) {
        return Expression(location, object->address->address);
    }
    else {
        return Expression(std::make_shared<ObjectExpression>(location, object));
    }
}

std::optional<Value> ObjectExpression::maximum_value() const {
    if (has_value()) {
        return value();
    }
    else {
        return Value(object->section->maximum_address() - object->size_range().minimum);
    }
}

std::optional<Value> ObjectExpression::minimum_value() const {
    if (has_value()) {
        return value();
    }
    else {
        return Value(object->section->minimum_address());
    }
}
