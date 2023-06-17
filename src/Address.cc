/*
Address.cc --

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

#include "Address.h"
#include "ExpressionParser.h"
#include "ParseException.h"

Address::Address(Tokenizer& tokenizer, const std::shared_ptr<Environment>& environment) {
    auto expression = ExpressionParser(tokenizer).parse();
    if (environment) {
        expression.evaluate(environment);
    }
    auto value = expression.value();
    if (!value.has_value() || !value->is_unsigned()) {
        throw ParseException(expression.location(), "constant unsigned integer expression expected");
    }

    auto token = tokenizer.next();

    if (token == Token::colon) {
        bank = value->unsigned_value();
        expression = ExpressionParser(tokenizer).parse();
        if (environment) {
            expression.evaluate(environment);
        }
        value = expression.value();
        if (!value.has_value() || !value->is_unsigned()) {
            throw ParseException(expression.location(), "constant unsigned integer expression expected");
        }
    }
    else {
        tokenizer.unget(token);
    }
    address = value->unsigned_value();
}


std::ostream& operator<<(std::ostream& stream, Address address) {
    address.serialize(stream);
    return stream;
}

void Address::serialize(std::ostream &stream) const {
    if (bank > 0) {
        stream << "$" << std::hex << bank << ":";
    }
    stream << "$" << std::hex << address;
}
