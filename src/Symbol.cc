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
const char* Symbol::empty_id = "";

Symbol::Symbol(const std::string &name) {
    init_global();
    auto it = global->symbols.find(name);
    if (it == global->symbols.end()) {
        auto length = name.length() + 1;
        char *interned_name = static_cast<char *>(malloc(length));
        strncpy(interned_name, name.c_str(), length);

        global->symbols[name] = interned_name;
        id = interned_name;
    }
    else {
        id = it->second;
    }
}


Symbol& Symbol::operator=(const std::string& name) {
    *this = Symbol(name);
    return *this;
}


std::ostream& operator<<(std::ostream& stream, const Symbol& symbol) {
    stream << symbol.str();
    return stream;
}

Symbol::Table::~Table() {
    for (auto& [string, interned_name]: symbols) {
        if (interned_name != empty_id) {
            free(interned_name);
        }
    }
}
