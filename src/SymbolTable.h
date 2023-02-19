/*
SymbolTable.h -- Map Strings to Integers

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

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>

typedef uint32_t symbol_t;

class SymbolTable {
public:
    SymbolTable() = default;
    explicit SymbolTable(const std::vector<std::string>& initial_symbols);
    symbol_t add(const std::string& name);
    bool contains(const std::string& name) {return symbols.find(name) != symbols.end();}
    symbol_t operator[](const std::string& name) const;
    const std::string& operator[](symbol_t symbol) const;

    [[nodiscard]] bool empty() const  {return names.empty();}
    [[nodiscard]] size_t size() const {return names.size();}

    static SymbolTable global;

    static bool global_less(symbol_t a, symbol_t b);

private:
    std::unordered_map<std::string, symbol_t> symbols;
    std::vector<std::string> names = {""};
};

#endif // SYMBOL_TABLE_H
