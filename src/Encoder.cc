/*
Encoding.cc --

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

#include "Encoder.h"

#include "Exception.h"
#include "Target.h"

Encoder::Encoder(const Value& value) {
    if (value.is_binary()) {
        encoder = std::make_shared<BinaryEncoder>();
    }
    else if (value.is_integer()) {
        encoder = std::make_shared<IntegerEncoder>(value);
    }
    else if (value.is_string()) {
        auto string_encoding = Target::current_target->default_string_encoding;
        if (!string_encoding) {
            throw Exception("no default string encoding specified");
        }
        encoder = std::make_shared<StringEncoder>(string_encoding);
    }
    else {
        throw Exception("can't encode %s", value.type_name().c_str());
    }
}


std::ostream& operator<<(std::ostream& stream, const Encoder& encoder) {
    encoder.serialize(stream);
    return stream;
}
