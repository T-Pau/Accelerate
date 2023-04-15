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

std::shared_ptr<Expression> ObjectExpression::clone() const {
    auto node = std::make_shared<ObjectExpression>(object);
    node->set_byte_size(byte_size());
    return node;
}

size_t ObjectExpression::minimum_byte_size() const {
    if (has_value()) {
        return Int::minimum_byte_size(value().unsigned_value());
    }
    else if (object->section != nullptr) {
        return object->section->address_size;
    }
    else {
        return 0;
    }
}

std::shared_ptr<Expression> ObjectExpression::evaluate(const Environment &environment) const {
    if (!had_value && object->has_address()) {
        // TODO: return ValueExpression
        return clone();
    }
    else {
        return {};
    }
}
