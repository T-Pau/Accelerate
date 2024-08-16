/*
Environment.h -- 

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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "Expression.h"
#include "Symbol.h"
#include "UnnamedLabelList.h"

class Function;
class Macro;

class Environment {
public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> next): next({std::move(next)}) {}
    Environment(const Environment& env) = default;

    void add(Symbol name, Expression value) {variables[name] = std::move(value);} // TODO: check for duplicates
    void add(Symbol name, const Function* function) {functions[name] = function;} // TODO: check for duplicates
    void add(Symbol name, const SizeRange& offset) {labels[name] = offset;} // TODO: check for duplicates
    void add(Symbol name, const Macro* macro) {macros[name] = macro;} // TODO: check for duplicates
    void add_next(std::shared_ptr<Environment> new_next) {next.emplace_back(std::move(new_next));}
    void clear_next() {next.clear();}
    [[nodiscard]] const std::unordered_map<Symbol, const Function*>& all_functions() const {return functions;}
    [[nodiscard]] const std::unordered_map<Symbol, SizeRange>& all_labels() const {return labels;}
    [[nodiscard]] const std::unordered_map<Symbol, const Macro*>& all_macros() const {return macros;}
    [[nodiscard]] const std::unordered_map<Symbol, Expression>& all_variables() const {return variables;}
    [[nodiscard]] bool empty() const {return functions.empty() && labels.empty() && macros.empty() && variables.empty();}
    [[nodiscard]] const Function* get_function(Symbol name) const;
    [[nodiscard]] std::optional<SizeRange> get_label(Symbol name) const;
    [[nodiscard]] const Macro* get_macro(Symbol name) const;
    [[nodiscard]] std::optional<Expression> get_variable(Symbol name) const {return (*this)[name];}
    void remove(Symbol name) {variables.erase(name);}
    void update(Symbol name, const SizeRange& offset) {labels[name] = offset;} // TODO: check for existence

    std::optional<Expression> operator[](Symbol name) const;

    void replace(const std::shared_ptr<Environment>& old_next, const std::shared_ptr<Environment>& new_next);

    UnnamedLabelList unnamed_labels;

private:
    std::unordered_map<Symbol, const Function*> functions;
    std::unordered_map<Symbol, SizeRange> labels;
    std::unordered_map<Symbol, const Macro*> macros;
    std::vector<std::shared_ptr<Environment>> next;
    std::unordered_map<Symbol, Expression> variables;
};


#endif // ENVIRONMENT_H
