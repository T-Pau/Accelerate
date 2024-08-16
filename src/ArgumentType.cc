/*
ArgumentType.cc -- 

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

#include "ArgumentType.h"

#include "Exception.h"

Value ArgumentTypeEnum::entry(Symbol name) const {
    auto it = entries.find(name);

    if (it == entries.end()) {
        throw Exception("invalid value for argument");
    }
    else {
        return it->second;
    }
}


Value ArgumentTypeMap::entry(const Value& value) const {
    auto it = entries.find(value);

    if (it == entries.end()) {
        throw Exception("invalid value for argument");
    }
    else {
        return it->second;
    }
}

const ArgumentTypeEncoding *ArgumentType::as_encoding() const {
    return dynamic_cast<const ArgumentTypeEncoding*>(this);
}

std::unique_ptr<ArgumentType> ArgumentTypeEncoding::range_type(Symbol range_name) const {
    auto range = std::make_unique<ArgumentTypeRange>(range_name);
    // TODO: handle encodings with unknown minimum/maximum value.
    range->lower_bound = *encoding.minimum_value();
    range->upper_bound = *encoding.minimum_value();

    return range;
}
