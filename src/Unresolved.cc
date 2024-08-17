/*
Unresolved.cc -- 

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

#include "Unresolved.h"

#include <algorithm>

#include "FileReader.h"

void Unresolved::Part::add(const Token& user, Symbol used) {
    const auto it = unresolved.find(used);

    if (it == unresolved.end()) {
        unresolved[used] = std::unordered_set<Token>{user};
    }
    else {
        it->second.insert(user);
    }
}

void Unresolved::Part::add(const Part& other) {
    unresolved.insert(other.unresolved.begin(), other.unresolved.end());
}

void Unresolved::add(const Token& user, const EvaluationResult& result) {
    for (auto& name : result.unresolved_functions) {
        functions.add(user, name);
    }
    for (auto& name : result.unresolved_macros) {
        macros.add(user, name);
    }
    for (auto& name : result.unresolved_variables) {
        variables.add(user, name);
    }
}

void Unresolved::add(const Unresolved& other) {
    functions.add(other.functions);
    macros.add(other.macros);
    variables.add(other.variables);
}

void Unresolved::clear() {
    functions.clear();
    macros.clear();
    variables.clear();
}

void Unresolved::Part::report() const {
    // TODO: sort case insensitive
    auto unresolved_symbols = std::vector<Symbol>{};
    for (auto& it : unresolved) {
        unresolved_symbols.push_back(it.first);
    }
    std::ranges::sort(unresolved_symbols);

    for (auto& unresolved_symbol : unresolved_symbols) {
        auto it = unresolved.find(unresolved_symbol);
        if (it == unresolved.end()) {
            throw Exception("internal error: unresolved symbol %s diapperead", unresolved_symbol.c_str());
        }
        auto& users = it->second;
        auto sorted_users = std::vector<Token>{users.begin(), users.end()};
        std::sort(sorted_users.begin(), sorted_users.end(), [](const Token& a, const Token& b) {
            return a.as_symbol() < b.as_symbol();
        });
        FileReader::global.error({}, "unresolved %s %s, referenced by:", type.c_str(), unresolved_symbol.c_str());
        for (auto& user : sorted_users) {
            auto location = user.location.to_string();
            if (location.empty()) {
                FileReader::global.error({}, "    %s", user.as_symbol().c_str());
            }
            else {
                FileReader::global.error({}, "    %s (%s)", user.as_symbol().c_str(), location.c_str());
            }
        }
    }
}

void Unresolved::report() const {
    functions.report();
    macros.report();
    variables.report();
}
