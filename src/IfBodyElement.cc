/*
IfBodyElement.cc --

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

#include "IfBodyElement.h"

std::shared_ptr<BodyElement> IfBodyElement::evaluate(const Environment &environment) const {
    auto new_clauses = std::vector<Clause>();
    auto changed = false;
    auto first = true;
    auto ignore = false;

    for (auto& clause: clauses) {
        if (ignore) {
            changed = true;
            break;
        }
        auto new_expression = clause.condition->evaluate(environment);
        if (new_expression) {
            changed = true;
        }
        else {
            new_expression = clause.condition;
        }
        auto new_body = BodyElement::evaluate(clause.body, environment);
        if (new_body) {
            changed = true;
        }
        else {
            new_body = clause.body;
        }

        if (new_body->empty()) {
            changed = true;
            continue;
        }

        if (new_expression->has_value()) {
            if (!*new_expression->value()) {
                continue;
            }
            else if (first) {
                return new_body;
            }
        }
        new_clauses.emplace_back(new_expression, new_body);

        if (new_expression->has_value() && *new_expression->value()) {
            ignore = true;
        }

        first = false;
    }

    if (changed) {
        return std::make_shared<IfBodyElement>(new_clauses);
    }
    else {
        return {};
    }
}

void IfBodyElement::serialize(std::ostream &stream) const {
    if (empty()) {
        return;
    }
    if (clauses.front()) {
        stream << clauses.front().body;
    }

    auto first = true;
    for (auto& clause: clauses) {
        if (clause) {
            stream << ".else" << std::endl;
        }
        else {
            if (first) {
                stream << ".if ";
            }
            else {
                stream << ".else if ";
            }
            stream << clause.condition << std::endl;
        }

        stream << clause.body;

        first = false;
    }

    stream << ".end" << std::endl;
}

std::optional<uint64_t> IfBodyElement::size() const {
    if (empty()) {
        return 0;
    }

    auto common_size = std::optional<uint64_t>();

    auto first = true;
    for (auto &clause: clauses) {
        auto current_size = clause.body->size();
        if (first) {
            common_size = current_size;
        }
        else {
            if (current_size != common_size) {
                return {};
            }
        }
        if (clause) {
            break;
        }

        first = false;
    }

    return common_size;
}


uint64_t IfBodyElement::maximum_size() const {
    if (empty()) {
        return 0;
    }

    auto common_size = uint64_t();

    auto first = true;
    for (auto &clause: clauses) {
        auto current_size = clause.body->minimum_size();
        if (first) {
            common_size = current_size;
        }
        else {
            common_size = std::max(common_size, current_size);
        }
        if (clause) {
            break;
        }

        first = false;
    }

    return common_size;
}

uint64_t IfBodyElement::minimum_size() const {
    if (empty()) {
        return 0;
    }

    auto common_size = uint64_t();

    auto first = true;
    for (auto &clause: clauses) {
        auto current_size = clause.body->minimum_size();
        if (first) {
            common_size = current_size;
        }
        else {
            common_size = std::min(common_size, current_size);
        }
        if (clause) {
            break;
        }

        first = false;
    }

    return common_size;
}

