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

#include "Exception.h"

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

const char* Symbol::Table::intern(const std::string& string) {
    if (string.empty()) {
        return empty_string.c_str();
    }
    auto it = symbols.find(string.c_str());
    if (it == symbols.end()) {
        auto interned_string = std::make_unique<std::string>(string);
        auto new_id = interned_string->c_str();
        names[new_id] = interned_string.get();
        symbols[new_id] = std::move(interned_string);
        return new_id;
    }
    else {
        return it->second->c_str();
    }
}

const std::string& Symbol::Table::recover(const char* interened) {
    if (interened == empty_string.c_str()) {
        return empty_string;
    }
    auto it = names.find(interened);
    if (it == names.end()) {
        throw Exception("internal error: unknown symbol '%s'", interened);
    }
    return *it->second;
}
