/*
IntegerExpression.cc -- 

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

#include "IntegerExpression.h"

#include "ParseException.h"

IntegerExpression::IntegerExpression(const Token &token) {
    if (!token.is_integer()) {
        throw ParseException(token, "internal error: can't create integer node from %s", token.type_name());
    }
    value_ = static_cast<int64_t>(token.as_integer()); // TODO: handle overflow
    set_byte_size(token.byte_size > 0 ? token.byte_size : Int::minimum_byte_size(value_));
}


std::shared_ptr<Expression> IntegerExpression::clone() const {
    auto node = std::make_shared<IntegerExpression>(value());
    node->set_byte_size(byte_size());
    return node;
}


void IntegerExpression::serialize_sub(std::ostream &stream) const {
    auto width = static_cast<int>(byte_size() > 0 ? byte_size() : minimum_byte_size()) * 2;
    if (value() < 0) {
        stream << "-";
    }
    stream << "$" << std::setfill('0') << std::setw(width) << std::hex << std::abs(value()) << std::dec;
}
