/*
EvaluationResult.cc --

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

#include "EvaluationResult.h"

#include "LabelExpression.h"

void EvaluationResult::set_unnamed_label(const std::shared_ptr<Label>& label) {
    previous_unnamed_label = label;
    previous_unnamed_label_valid = true;
    for (auto expression: forward_unnamed_label_uses) {
        expression->set_label(label);
    }
    forward_unnamed_label_uses.clear();
}

void EvaluationResult::invalidate_unnamed_label() {
    previous_unnamed_label.reset();
    previous_unnamed_label_valid = false;
    forward_unnamed_label_uses.clear();
}

std::shared_ptr<Label> EvaluationResult::get_previous_unnamed_label() const {
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
