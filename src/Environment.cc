/*
Environment.cc -- 

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

#include "Environment.h"

std::optional<Expression> Environment::operator[](Symbol name) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    for (auto& environment: next) {
        auto value = (*environment)[name];
        if (value) {
            return value;
        }
    }

    return {};
}

void Environment::replace(const std::shared_ptr<Environment>& old_next, const std::shared_ptr<Environment>& new_next) {
    for (auto& environment: next) {
        if (environment == old_next) {
            environment = new_next;
            return;
        }
    }
}

const Function* Environment::get_function(Symbol name) const {
    auto it = functions.find(name);

    if (it != functions.end()) {
        return it->second;
    }
    for (auto& environment: next) {
        auto function = environment->get_function(name);
        if (function) {
            return function;
        }
    }
    return nullptr;
}

const Macro* Environment::get_macro(Symbol name) const {
    auto it = macros.find(name);

    if (it != macros.end()) {
        return it->second;
    }
    for (auto& environment: next) {
        auto macro = environment->get_macro(name);
        if (macro) {
            return macro;
        }
    }
    return nullptr;
}

