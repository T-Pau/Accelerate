#ifndef HAD_XLR8_ASSIGNMENT_BODY_H
#define HAD_XLR8_ASSIGNMENT_BODY_H

/*
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

#include "Body.h"

/**
 * @brief Represents an assignment, which defines a local constant or variable: `name = value`.
 */
class AssignmentBody: public BodyElement {
public:
    /**
     * Create an assignment body.
     * 
     * @param visibility The visibility of the assignment.
     * @param name The name of the variable or constant being assigned.
     * @param value The value being assigned.
     */
    static Body create(Visibility visibility, Symbol name, Expression value) {
        return Body(std::make_shared<AssignmentBody>(visibility, name, std::move(value)));
    }

    AssignmentBody(Visibility visibility, Symbol name, Expression value): visibility(visibility), name(name), value(std::move(value)) {}

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<AssignmentBody>(visibility, name, value);}
    [[nodiscard]] bool empty() const override {return false;}
    void encode(std::string &bytes, const Memory *memory) const override {return;}
    void serialize(std::ostream &stream, const std::string &prefix) const override;
    void resolve(Scope* scope, Entity* containing_entity) override {value.resolve(scope, containing_entity);}
    void expand_calls() override {value.expand_calls();}
    void enter_names(Scope* scope, Entity* containing_entity) override;

private:
    Visibility visibility;
    Symbol name;
    Expression value;
};


#endif // HAD_XLR8_ASSIGNMENT_BODY_H
