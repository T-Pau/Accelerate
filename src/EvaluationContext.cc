//
// Created by Dieter Baron on 15.05.23.
//

#include "EvaluationContext.h"

#include <utility>

#include "Exception.h"
#include "Object.h"

EvaluationContext EvaluationContext::evaluating_variable(Symbol variable) const {
    auto new_context = *this;
    new_context.evaluating_variables.insert(variable);
    return new_context;
}

EvaluationContext EvaluationContext::adding_offset(SizeRange size) const {
    auto new_context = *this;
    new_context.offset += size;
    return new_context;
}

EvaluationContext EvaluationContext::adding_scope(std::shared_ptr<Environment> new_environment) const {
    auto new_context = *this;
    new_context.environment = std::move(new_environment);
    new_context.scope = std::make_shared<EvaluationScope>(new_context.scope);
    return new_context;
}

EvaluationContext EvaluationContext::setting_offset(SizeRange new_offset) const {
    auto new_context = *this;
    new_context.offset = new_offset;
    return new_context;
}

EvaluationContext EvaluationContext::making_conditional() const {
    auto new_context = *this;
    new_context.conditional = true;
    new_context.conditional_in_scope = true;
    return new_context;
}

EvaluationContext::EvaluationContext(Object* object, std::shared_ptr<Environment> environment_, SizeRange offset, bool conditional, bool shallow): environment(std::move(environment_)), object(object), shallow(shallow), offset(offset), conditional(conditional), conditional_in_scope(conditional) {
    if (!environment) {
        if (object) {
            environment = object->environment;
        }
        else {
            environment = std::make_shared<Environment>();
        }
    }
}
