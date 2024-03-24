/*
EvaluationContext.h --

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

#ifndef EVALUATION_CONTEXT_H
#define EVALUATION_CONTEXT_H

#include <unordered_set>

#include "Environment.h"
#include "EvaluationResult.h"
#include "Memory.h"
#include "SizeRange.h"

class Entity;
class ObjectFile;

class EvaluationContext {
  public:
    enum EvaluationType {
        ARGUMENTS,          // Bind arguments in function call or instruction encoding.
        ENTITY,             // Evaluate object with completed body.
        MACRO_EXPANSION,    // Bind arguments and duplicate labels in macro invocation.
        STANDALONE          // Preprocessor if condition.
    };

    // ARGUMENTS, MACRO_EXPANSION, STANDALONE
    EvaluationContext(EvaluationResult& result, EvaluationType type, std::shared_ptr<Environment> environment, std::unordered_set<Symbol> defines = {}, SizeRange offset = SizeRange(0, {})): type(type), environment(std::move(environment)), defines{std::move(defines)}, result(result), offset(offset) {}
    // ENTITY
    EvaluationContext(EvaluationResult& result, Entity* entity);

    EvaluationContext(const EvaluationContext& context) = default;

    [[nodiscard]] bool shallow() const;

    [[nodiscard]] EvaluationContext evaluating_variable(Symbol variable) const;
    [[nodiscard]] EvaluationContext skipping_variables(const std::vector<Symbol>& variables) const;
    [[nodiscard]] EvaluationContext adding_offset(SizeRange size) const;
    [[nodiscard]] EvaluationContext setting_offset(SizeRange offset) const;
    [[nodiscard]] EvaluationContext making_conditional() const;
    [[nodiscard]] bool evaluating(Symbol variable) const {return evaluating_variables.contains(variable);}
    [[nodiscard]] bool skipping(Symbol variable) const {return skip_variables.contains(variable);}
    [[nodiscard]] std::optional<Expression> lookup_variable(Symbol variable) const;

    EvaluationType type;
    Entity* entity = nullptr;
    std::shared_ptr<Environment> environment;
    std::unordered_set<Symbol> defines;
    SizeRange offset = SizeRange(0, {});
    bool conditional = false;
    std::unordered_set<Symbol> skip_variables; // For preserving arguments in macro/function bodies.
    std::unordered_set<Symbol> evaluating_variables; // Variables currently being evaluated (used for circular definition detection).
    EvaluationResult& result;
};

#endif // EVALUATION_CONTEXT_H
