//
// Created by Dieter Baron on 15.05.23.
//

#include "EvaluationContext.h"

EvaluationContext::EvaluationContext(const EvaluationContext &context, Symbol variable): EvaluationContext(context) {
    evaluating_variables.insert(variable);
}

EvaluationContext::EvaluationContext(const EvaluationContext &context, SizeRange new_offset): EvaluationContext(context) {
    offset = new_offset;
}

EvaluationContext::EvaluationContext(const EvaluationContext &context, bool new_top_level): EvaluationContext(context) {
    top_level = new_top_level;
}
