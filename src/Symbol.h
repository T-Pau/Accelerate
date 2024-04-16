/*
Symbol.h -- Map Strings to Integers

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

#ifndef SYMBOL_H
#define SYMBOL_H

#include <cstring>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class Symbol {
public:
    Symbol() = default;
    explicit Symbol(const std::string& name);
    Symbol& operator=(const std::string& name);


    [[nodiscard]] std::string str() const {return {id};}
    [[nodiscard]] const char* c_str() const {return id;}
    [[nodiscard]] bool empty() const {return id==empty_id;}

    bool operator==(const Symbol& other) const {return id == other.id;}
    bool operator!=(const Symbol& other) const {return id != other.id;}
    bool operator<(const Symbol& other) const {return strcmp(id, other.id) < 0;}
    bool operator<=(const Symbol& other) const {return strcmp(id, other.id) <= 0;}
    bool operator>(const Symbol& other) const {return strcmp(id, other.id) > 0;}
    bool operator>=(const Symbol& other) const {return strcmp(id, other.id) >= 0;}
    operator bool() const {return !empty();} // NOLINT(*-explicit-constructor)

private:
    const char* id{empty_id};

    class Table {
      public:
        ~Table();

        std::unordered_map<std::string, char*> symbols = {{"", const_cast<char*>(empty_id)}};
        std::vector<std::string> names = {""};
    };

    static const char* empty_id;

    static void init_global() {if (!global) {global = new Table();}}
    static Table* global;
};

template<>
struct std::hash<Symbol>
{
    std::size_t operator()(Symbol const& symbol) const noexcept {
        return std::hash<const char*>{}(symbol.c_str());
    }
};

std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);

#endif // SYMBOL_H
