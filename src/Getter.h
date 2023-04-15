/*
Getter.h -- 

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#ifndef GETTER_H
#define GETTER_H

#include <string>

#include "Exception.h"
#include "Path.h"
#include "Symbol.h"

template <typename T>
class Getter {
public:
    const T& get(Symbol name, Symbol base = {}) {
        auto base_filename = name.str();
        if (!base_filename.ends_with(filename_extension())) {
            base_filename += filename_extension();
        }
        auto filename = path->find(Symbol(base_filename), base);
        if (filename.empty()) {
            throw Exception("cannot find %s", base_filename.c_str());
        }

        auto it = instances.find(filename);
        if (it != instances.end()) {
            return it->second;
        }
        auto pair = instances.insert({filename, parse(name, filename)});
        if (!pair.second) {
            throw Exception("internal error: can't insert '%s'", name.c_str());
        }
        return pair.first->second;
    }

    const T& get(const std::string& name) {return get(Symbol(name));}

    std::shared_ptr<Path> path = std::make_shared<Path>();

protected:
    [[nodiscard]] virtual std::string filename_extension() const = 0;
    virtual T parse(Symbol name, Symbol filename) = 0;

private:
    std::unordered_map<Symbol, T> instances;
};

#endif // GETTER_H
