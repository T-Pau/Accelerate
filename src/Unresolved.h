/*
Unresolved.h --

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

#ifndef UNRESOLVED_H
#define UNRESOLVED_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "EvaluationResult.h"
#include "Symbol.h"

class Entity;

class Unresolved {
  public:
    class Part {
      public:
        class User {
          public:
            User(const Entity* entity);

            User(Symbol name, const Location& location) : name{name}, location{location} {}

            bool operator==(const User &other) const {return name == other.name && location == other.location;}

            Symbol name;
            Location location;
        };

        Part(std::string type) : type{std::move(type)} {}

        void add(const Entity* user, Symbol used) { add(User{user}, used); }

        void add(Symbol name, const Location& location, Symbol used) { add(User{name, location}, used); }

        void add(const User& user, Symbol used);
        void add(const Part& other);

        void clear() { unresolved.clear(); }

        [[nodiscard]] bool empty() const { return unresolved.empty(); }

        void report() const;

      private:
        std::string type;
        std::unordered_map<Symbol, std::unordered_set<User>> unresolved;
    };

    void add(const Entity* user, const EvaluationResult& result) { add(Part::User{user}, result); }

    void add(Symbol name, const Location& location, const EvaluationResult& result) { add(Part::User{name, location}, result); }

    void add(const Unresolved& other);
    void clear();

    [[nodiscard]] bool empty() const { return functions.empty() && macros.empty() && variables.empty(); }

    void report() const;

    Part functions{"function"};
    Part macros{"macro"};
    Part variables{"variable"};

  private:
    void add(const Part::User& user, const EvaluationResult& result);
};

template <> struct std::hash<Unresolved::Part::User> {
    std::size_t operator()(const Unresolved::Part::User& user) const noexcept {
        return std::hash<Symbol>()(user.name) ^ (std::hash<Location>()(user.location) << 1);
    }
};

#endif // UNRESOLVED_H
