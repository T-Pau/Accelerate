//
// Created by Dieter Baron on 21.05.23.
//

#ifndef EVALUATION_SCOPE_H
#define EVALUATION_SCOPE_H

#include <unordered_set>

#include "Label.h"

class LabelExpression;

class EvaluationScope {
public:
    EvaluationScope() = default;
    explicit EvaluationScope(std::shared_ptr<EvaluationScope> next): next(std::move(next)) {}

    std::shared_ptr<Label> previous_unnamed_label;
    bool previous_unnamed_label_valid = true;
    std::unordered_set<LabelExpression *> forward_unnamed_label_uses;

    std::shared_ptr<EvaluationScope> next;
};

#endif // EVALUATION_SCOPE_H
