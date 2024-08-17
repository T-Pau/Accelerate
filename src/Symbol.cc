/*
Symbol.cc -- Map Strings to Integers

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

#include "Symbol.h"

Symbol::Table* Symbol::global = nullptr;
const std::string Symbol::empty_string{};

Symbol::Symbol(const std::string &name) {
    init_global();
    id = global->intern(name);
}

Symbol& Symbol::operator=(const std::string& name) {
    *this = Symbol(name);
    return *this;
}


std::ostream& operator<<(std::ostream& stream, const Symbol& symbol) {
    stream << symbol.c_str();
    return stream;
}

void Symbol::init_global() {
    if (!global) {
        global = new Symbol::Table();
    }
}

const std::string* Symbol::Table::intern(const std::string& string) {
    if (string.empty()) {
        return &empty_string;
    }
    auto it = symbols.find(&string);
    if (it == symbols.end()) {
        auto interned_string = std::make_unique<std::string>(string);
        auto [new_it, _] = symbols.insert({interned_string.get(), std::move(interned_string)});
        return new_it->first;
    }
    else {
        return it->second.get();
    }
}
