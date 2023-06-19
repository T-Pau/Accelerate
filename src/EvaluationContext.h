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
#include <utility>

#include "Environment.h"
#include "EvaluationResult.h"
#include "Memory.h"
#include "SizeRange.h"

class ObjectFile;

class EvaluationContext {
public:
    explicit EvaluationContext(EvaluationResult& result): environment(std::make_shared<Environment>()), result(result) {}
    EvaluationContext(EvaluationResult& result, std::shared_ptr<Environment> environment, bool shallow = false, bool expanding_macro = false): environment(std::move(environment)), shallow(shallow), expanding_macro(expanding_macro), result(result) {}
    EvaluationContext(EvaluationResult& result, Object* object, std::shared_ptr<Environment> environment, SizeRange offset, bool conditional = false, bool shallow = false);
    EvaluationContext(const EvaluationContext& context) = default;

    [[nodiscard]] EvaluationContext evaluating_variable(Symbol variable) const;
    [[nodiscard]] EvaluationContext adding_offset(SizeRange size) const;
    [[nodiscard]] EvaluationContext adding_scope(std::shared_ptr<Environment> environment, std::unordered_map<Label*, std::shared_ptr<Label>> remap_labels = {}, std::unordered_map<BaseExpression*, Expression> remap_expressions = {}) const;
    [[nodiscard]] EvaluationContext setting_offset(SizeRange offset) const;
    [[nodiscard]] EvaluationContext making_conditional() const;
    [[nodiscard]] bool evaluating(Symbol variable) const {return evaluating_variables.contains(variable);}

    std::shared_ptr<Environment> environment;
    Object* object = nullptr;
    bool shallow = false; // Don't evaluate values of variables (used in macros and functions for parameters).
    bool expanding_macro = false;
    std::unordered_map<BaseExpression*, Expression> remap_expressions;
    std::unordered_map<Label*, std::shared_ptr<Label>> remap_labels;

    SizeRange offset;
    bool conditional = false;
    bool conditional_in_scope = false;
    std::unordered_set<Symbol> evaluating_variables; // Variables currently being evaluated (used for loop detection).
    EvaluationResult& result;
};

#endif // EVALUATION_CONTEXT_H
