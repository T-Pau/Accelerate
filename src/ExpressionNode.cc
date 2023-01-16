/*
ExpressionNode.cc -- Abstract Base Class of Expression Nodes

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

#include "ExpressionNode.h"
#include "ParseException.h"

std::shared_ptr<ExpressionNode> ExpressionNode::parse(Tokenizer &tokenizer) {
    auto token = tokenizer.next();

    // TODO: implement

    switch (token.get_type()) {
        case Token::INTEGER:
            return std::make_shared<ExpressionNodeInteger>(token);

        default:
            return {};
    }
}

ExpressionNodeInteger::ExpressionNodeInteger(const Token &token) {
    if (!token.is_integer()) {
        throw ParseException(token, "internal error: can't create integer node from %s", token.type_name());
    }
    value = static_cast<int64_t>(token.as_integer()); // TODO: handle overflow
}

size_t ExpressionNodeInteger::byte_size() const {
    return 0; // TODO
}

size_t ExpressionNodeInteger::minimum_size() const {
    if (value > std::numeric_limits<uint32_t>::max()) {
        return 8;
    }
    else if (value > std::numeric_limits<uint16_t>::max()) {
        return 4;
    }
    else if (value > std::numeric_limits<uint8_t>::max()) {
        return 2;
    }
    else {
        return 1;
    }
}

ExpressionNodeVariable::ExpressionNodeVariable(const Token &token) {
    if (!token.is_name()) {
        throw Exception("internal error: creating ExpressionNodeVariable from non-name token");
    }
    location = token.location;
    symbol = token.as_symbol();
}
