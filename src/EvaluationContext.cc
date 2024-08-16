/*
EvaluationContext.cc --

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

#include "EvaluationContext.h"

#include "Entity.h"

EvaluationContext::EvaluationContext(EvaluationResult& result, EvaluationType type, std::shared_ptr<Environment> environment, std::unordered_set<Symbol> defines, const SizeRange& offset): type(type), environment(std::move(environment)), defines{std::move(defines)}, offset(offset), result(result) {
    if (type == MACRO_EXPANSION) {
        label_offset = SizeRange(0, {});
        labels_are_offset = true;
    }
}

EvaluationContext::EvaluationContext(EvaluationResult& result, Entity* entity): type(ENTITY), entity(entity), environment(entity->environment), offset(0), result(result) {}

EvaluationContext EvaluationContext::evaluating_variable(Symbol variable) const {
    auto new_context = *this;
    new_context.evaluating_variables.insert(variable);
    return new_context;
}

EvaluationContext EvaluationContext::adding_offset(const SizeRange& size) const {
    auto new_context = *this;
    new_context.offset += size;
    return new_context;
}


EvaluationContext EvaluationContext::setting_offset(const SizeRange& new_offset) const {
    auto new_context = *this;
    new_context.offset = new_offset;
    return new_context;
}

EvaluationContext EvaluationContext::making_conditional() const {
    auto new_context = *this;
    new_context.conditional = true;
    return new_context;
}

bool EvaluationContext::shallow() const {
    switch (type) {
        case ARGUMENTS:
        case LABELS:
        case LABELS_2:
        case MACRO_EXPANSION:
            return true;

        case ENTITY:
        case OUTPUT:
        case STANDALONE:
            return false;
    }

    throw Exception("internal error: invalid evaluation type");
}

EvaluationContext EvaluationContext::skipping_variables(const std::vector<Symbol>& variables) const {
    auto new_context = *this;
    new_context.skip_variables.insert(variables.begin(), variables.end());
    return new_context;
}

std::optional<Expression> EvaluationContext::lookup_variable(Symbol variable) const {
    if (skipping(variable)) {
        return {};
    }
    return (*environment)[variable];
}

EvaluationContext EvaluationContext::adding_scope(std::shared_ptr<Environment> new_environment, const SizeRange& new_label_offset) const {
    auto new_context = *this;
    new_context.environment = std::move(new_environment);
    new_context.label_offset = new_label_offset;
    new_context.labels_are_offset = true;
    return new_context;
}

EvaluationContext EvaluationContext::keeping_label_offsets() const {
    auto new_context = *this;
    new_context.keep_label_offsets = true;
    return new_context;
}
