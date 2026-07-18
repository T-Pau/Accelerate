/*
Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "BodyElement.h"

#include "Body.h"

std::ostream& operator<<(std::ostream& stream, const BodyElement& element) {
    element.serialize(stream, "");
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BodyElement>& element) {
    stream << *element;
    return stream;
}

std::pair<bool, std::optional<Body>> BodyElement::append_sub(const Body& body, const Body& element) { return {false, {}}; }

void BodyElement::resolve(Scope* scope, Entity* containing_entity) {
    traverse([&](Body& sub_body) { sub_body.resolve(scope, containing_entity); }, [&](Expression& sub_expression) { sub_expression.resolve(scope, containing_entity); });
}

void BodyElement::expand_calls() {
    traverse([&](Body& sub_body) { sub_body.expand_calls(); }, [&](Expression& sub_expression) { sub_expression.expand_calls(); });
}

std::optional<Body> BodyElement::evaluate(const EvaluationContext& context) {
    traverse([&](Body& sub_body) { sub_body.evaluate(context); }, [&](Expression& sub_expression) { sub_expression.evaluate(context); });

    return evaluate_process(context);
}

// This can't be in the header file because it needs the full definition of Body.
std::optional<Body> BodyElement::evaluate_process(const EvaluationContext& context) { return {}; }

void BodyElement::enter_names(Scope* scope, Entity* containing_entity) {
    traverse([&](Body& sub_body) { sub_body.enter_names(scope, containing_entity); },
             [&](Expression& sub_expression) {
                 // Expressions can't define names.
             });
}

bool BodyElement::fully_evaluated() {
    try {
        traverse(
            [&](Body& sub_body) {
                if (!sub_body.fully_evaluated()) {
                    throw NotFullyEvaluatedException();
                }
            },
            [&](Expression& sub_expression) {
                if (!sub_expression.has_value()) {
                    throw NotFullyEvaluatedException();
                }
            });
        return true;
    } catch (const NotFullyEvaluatedException&) {
        return false;
    }
}
