/*
SymbolTable.cc -- Map Strings to Integers

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

#include "SymbolTable.h"

#include "Exception.h"

SymbolTable SymbolTable::global;

SymbolTable::SymbolTable(const std::vector<std::string> &initial_symbols) {
    for (const auto& symbol: initial_symbols) {
        add(symbol);
    }
}

symbol_t SymbolTable::add(const std::string &name) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        auto symbol = names.size();
        names.push_back(name);
        symbols[name] = symbol;
        return symbol;
    }
    else {
        return it->second;
    }
}

symbol_t SymbolTable::operator[](const std::string &name) const {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        throw Exception("unknown symbol '%s'", name.c_str());
    }
    return it->second;
}

const std::string &SymbolTable::operator[](symbol_t symbol) const {
    if (symbol >= names.size()) {
        throw Exception("unknown symbol %u", symbol);
    }
    return names[symbol];
}

bool SymbolTable::global_less(symbol_t a, symbol_t b) {
    return SymbolTable::global[a] < SymbolTable::global[b];
}
