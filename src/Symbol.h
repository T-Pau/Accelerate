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

#include <string>
#include <unordered_map>
#include <vector>

class Symbol {
public:
    Symbol(): id(0) {}
    explicit Symbol(const std::string& name);
    Symbol& operator=(const std::string& name);


    [[nodiscard]] const std::string& str() const {return names[id];}
    [[nodiscard]] const char* c_str() const {return str().c_str();}
    [[nodiscard]] uint32_t value() const {return id;}
    [[nodiscard]] bool empty() const {return id==0;}

    bool operator==(const Symbol& other) const {return id == other.id;}
    bool operator!=(const Symbol& other) const {return id != other.id;}
    bool operator<(const Symbol& other) const {return str() < other.str();}
    bool operator<=(const Symbol& other) const {return str() <= other.str();}
    bool operator>(const Symbol& other) const {return str() > other.str();}
    bool operator>=(const Symbol& other) const {return str() >= other.str();}

private:
    uint32_t id;

    static std::unordered_map<std::string, uint32_t> symbols;
    static std::vector<std::string> names;
};

template<>
struct std::hash<Symbol>
{
    std::size_t operator()(Symbol const& symbol) const noexcept {
        return std::hash<uint32_t>{}(symbol.value());
    }
};

std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);

#endif // SYMBOL_H
