/*
IfBodyElement.h --

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

#ifndef IF_BODY_ELEMENT_H
#define IF_BODY_ELEMENT_H

#include "BodyElement.h"
#include "Body.h"
#include "BaseExpression.h"
#include "Exception.h"

class IfBodyElement: public BodyElement {
public:
    class Clause {
    public:
        Clause(Expression condition, Body body): condition(std::move(condition)), body(std::move(body)) {}

        explicit operator bool() const {return condition.has_value() && *condition.value();}
        Expression condition;
        Body body;
    };

    IfBodyElement() = default;
    explicit IfBodyElement(std::vector<Clause> clauses);

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<IfBodyElement>(clauses);} // TODO: this doesn't copy clauses
    void collect_objects(std::unordered_set<Object*> &objects) const override;
    [[nodiscard]] bool empty() const override {return clauses.empty();}
    void encode(std::string &bytes, const Memory* memory) const override {throw Exception("unresolved if");}
    [[nodiscard]] std::optional<Body> evaluated(const Environment &environment, const SizeRange& offset) const override;

    void serialize(std::ostream &stream, const std::string& prefix) const override;

private:
    std::vector<Clause> clauses;
};


#endif // IF_BODY_ELEMENT_H
