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


Value ArgumentTypeMap::entry(Value value) const {
    auto it = entries.find(value);

    if (it == entries.end()) {
        throw Exception("invalid value for argument");
    }
    else {
        return it->second;
    }
}

std::optional<bool> ArgumentTypeMap::fits(const std::shared_ptr<BaseExpression> &expression) const {
    if (expression->has_value()) {
        return has_entry(*expression->value());
    }
    else {
        return {};
    }
}

std::optional<bool> ArgumentTypeRange::fits(const std::shared_ptr<BaseExpression> &expression) const {
    auto minimum_value = expression->minimum_value();
    auto maximum_value = expression->maximum_value();

    if (!minimum_value.has_value() || !maximum_value.has_value()) {
        return {};
    }
    else if (*maximum_value < lower_bound || *minimum_value > upper_bound) {
        return false;
    }
    else if (*minimum_value >= lower_bound && *maximum_value <= upper_bound) {
        return true;
    }
    else {
        return {};
    }
}
