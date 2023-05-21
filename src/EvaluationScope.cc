//
// Created by Dieter Baron on 21.05.23.
//

#include "EvaluationScope.h"

#include "LabelExpression.h"

void EvaluationScope::set_unnamed_label(const std::shared_ptr<Label>& label) {
    previous_unnamed_label = label;
    previous_unnamed_label_valid = true;
    for (auto expression: forward_unnamed_label_uses) {
        expression->set_label(label);
    }
    forward_unnamed_label_uses.clear();
}

void EvaluationScope::invalidate_unnamed_label() {
    previous_unnamed_label.reset();
    previous_unnamed_label_valid = false;
    forward_unnamed_label_uses.clear();
}

std::shared_ptr<Label> EvaluationScope::get_previous_unnamed_label() const {
    if (previous_unnamed_label_valid) {
        if (previous_unnamed_label) {
            return previous_unnamed_label;
        }
        else {
            throw Exception("no previous unnamed label");
        }
    }
    else {
        return {};
    }
}
