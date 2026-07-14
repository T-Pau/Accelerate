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

#include "IfBody.h"

#include "ErrorBody.h"

IfBody::IfBody(std::vector<IfBodyClause> clauses_): clauses(std::move(clauses_)) {
    if (!clauses.empty()) {
        size_range_ = SizeRange(std::numeric_limits<uint64_t>::max(), 0);
        for (const auto& clause: clauses) {
            if (!clause.body.is<ErrorBody>()) {
                size_range_.minimum = std::min(size_range_.minimum, clause.body.size_range().minimum);
                size_range_.maximum = std::max(size_range_.maximum, clause.body.size_range().maximum);
            }
        }
    }
}

void IfBody::serialize(std::ostream &stream, const std::string& prefix) const {
    if (empty()) {
        return;
    }
    if (clauses.front().is_true()) {
        clauses.front().body.serialize(stream, prefix);
        return;
    }

    auto first = true;
    for (auto& clause: clauses) {
        if (clause.is_true()) {
            stream << prefix << "}" << std::endl;
            stream << prefix << ".else {" << std::endl;
        }
        else {
            if (first) {
                stream << prefix << ".if ";
            }
            else {
                stream << prefix << "}" << std::endl;
                stream << prefix << ".else_if ";
            }
            stream << *clause.condition << " {" << std::endl;
        }

        clause.body.serialize(stream, prefix + "  ");

        first = false;
    }

    stream << prefix << "}" << std::endl;
}

std::optional<Body> IfBody::simplify(std::vector<IfBodyClause>& clauses, bool always_create) {
    auto filtered_clauses = std::vector<IfBodyClause>();

    for (auto& clause: clauses) {
        if (clause.is_false()) {
            continue;
        }

        filtered_clauses.emplace_back(clause);

        if (clause.is_true()) {
            if (filtered_clauses.size() == 1) {
                return clause.body;
            }
            else {
                break;
            }
        }
    }

    if (filtered_clauses.empty()) {
        return Body();
    }

    if (always_create && filtered_clauses.size() != clauses.size()) {
        return Body(std::make_shared<IfBody>(filtered_clauses));
    }
    else {
        return {};
    }
}


void IfBody::traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    for (auto& clause: clauses) {
        if (clause.condition) {
            expression_callable(*clause.condition);
        }
        body_callable(clause.body);
    }
}