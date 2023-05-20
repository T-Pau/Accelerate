//
// Created by Dieter Baron on 15.05.23.
//

#ifndef EVALUATION_CONTEXT_H
#define EVALUATION_CONTEXT_H

#include <unordered_set>
#include <utility>

#include "Environment.h"
#include "Memory.h"
#include "SizeRange.h"

class EvaluationContext {
public:
    EvaluationContext(): environment(empty_environment) {}
    explicit EvaluationContext(const Environment& environment, bool shallow = false): environment(environment), shallow(shallow) {}
    explicit EvaluationContext(const Environment& environment, bool top_level, SizeRange offset): environment(environment), top_level(top_level), offset(offset) {}
    EvaluationContext(const EvaluationContext& context, Symbol variable);
    EvaluationContext(const EvaluationContext& context, SizeRange offset);
    EvaluationContext(const EvaluationContext& context, bool top_level);
    EvaluationContext(const EvaluationContext& context) = default;

    [[nodiscard]] bool evaluating(Symbol variable) const {return evaluating_variables.contains(variable);}

    const Environment& environment;
    bool shallow = false; // Don't evaluate values of variables (used in macros and functions for parameters).

    SizeRange offset;
    bool top_level = false;
    std::unordered_set<Symbol> evaluating_variables; // Variables currently being evaluated (used for loop detection).

private:
    static Environment empty_environment;
};

#endif // EVALUATION_CONTEXT_H
