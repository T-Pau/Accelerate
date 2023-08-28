/*
ValueExpression.cc --

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

#include "ValueExpression.h"

#include "ParseException.h"

ValueExpression::ValueExpression(const Token &token) {
    if (token.is_value()) {
        value_ = token.as_value();
    }
    else if (token.is_string()) {
        value_ = Value(token.as_symbol());
    }
    else {
        throw ParseException(token, "internal error: can't create value node from %s", token.type_name());
    }
}


void ValueExpression::serialize_sub(std::ostream &stream) const {
    switch (value()->type()) {
        case Value::BOOLEAN:
            stream << (value()->boolean_value() ? "true" : "false");
            break;

        case Value::FLOAT:
            stream << value()->float_value();
            break;

        case Value::SIGNED:
        case Value::UNSIGNED: {
            auto width = static_cast<int>(value()->default_size()) * 2;
            uint64_t v;
            if (value()->is_signed()) {
                stream << "-";
                v = -value()->signed_value();
            }
            else {
                v = value()->unsigned_value();
            }
            stream << "$" << std::setfill('0') << std::setw(width) << std::hex << v << std::dec;
            break;
        }

        case Value::STRING:
            stream << '"' << value()->string_value() << '"';
            break;

        case Value::VOID:
            stream << "void";
            break;
    }
}

std::optional<Value> ValueExpression::maximum_value() const {
    if (value_.is_number()) {
        return value_;
    }
    else {
        return {};
    }
}