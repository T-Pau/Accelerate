//
// Created by Dieter Baron on 15.05.23.
//

#ifndef EVALUATION_CONTEXT_H
#define EVALUATION_CONTEXT_H

#include <unordered_set>
#include <utility>

#include "Environment.h"
#include "EvaluationScope.h"
#include "Memory.h"
#include "SizeRange.h"

class EvaluationContext {
public:
    EvaluationContext(): environment(std::make_shared<Environment>()) {}
    explicit EvaluationContext(std::shared_ptr<Environment> environment, bool shallow = false): environment(std::move(environment)), shallow(shallow) {}
    explicit EvaluationContext(Symbol object_name, std::shared_ptr<Environment> environment, bool conditional, SizeRange offset): environment(std::move(environment)), object_name(object_name), conditional(conditional), conditional_in_scope(conditional), offset(offset) {}
    EvaluationContext(const EvaluationContext& context) = default;

    [[nodiscard]] EvaluationContext evaluating_variable(Symbol variable) const;
    [[nodiscard]] EvaluationContext adding_offset(SizeRange size) const;
    [[nodiscard]] EvaluationContext adding_scope(std::shared_ptr<Environment> environment) const;
    [[nodiscard]] EvaluationContext setting_offset(SizeRange offset) const;
    [[nodiscard]] EvaluationContext making_conditional() const;
    [[nodiscard]] bool evaluating(Symbol variable) const {return evaluating_variables.contains(variable);}

    std::shared_ptr<Environment> environment;
    bool shallow = false; // Don't evaluate values of variables (used in macros and functions for parameters).
    Symbol object_name;

    SizeRange offset;
    bool conditional = false;
    bool conditional_in_scope = false;
    std::unordered_set<Symbol> evaluating_variables; // Variables currently being evaluated (used for loop detection).
    std::shared_ptr<EvaluationScope> scope = std::make_shared<EvaluationScope>();
};

#endif // EVALUATION_CONTEXT_H
