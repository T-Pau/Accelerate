/*
Environment.h -- 

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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <memory>
#include <utility>

#include "SymbolTable.h"

class ExpressionNode;

class Environment {
public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> next): next(std::move(next)) {}

    void add(symbol_t name, std::shared_ptr<ExpressionNode> value) {variables[name] = std::move(value);}
    void set(symbol_t name, const std::shared_ptr<ExpressionNode>& value);
    std::shared_ptr<ExpressionNode> operator[](symbol_t name) const;

private:
    bool update(symbol_t name, const std::shared_ptr<ExpressionNode>& value);

    std::unordered_map<symbol_t, std::shared_ptr<ExpressionNode>> variables;
    std::shared_ptr<Environment> next;
};


#endif // ENVIRONMENT_H