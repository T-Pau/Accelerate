//
// Created by Dieter Baron on 27.05.23.
//

#ifndef ASSIGNMENT_BODY_H
#define ASSIGNMENT_BODY_H

#include "Body.h"
#include "Object.h"

class AssignmentBody: public BodyElement {
public:
    AssignmentBody(Visibility visibility, Symbol name, Expression value): visibility(visibility), name(name), value(std::move(value)) {}

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<AssignmentBody>(visibility, name, value);}
    [[nodiscard]] bool empty() const override {return false;}
    void encode(std::string &bytes, const Memory *memory) const override {return;}
    [[nodiscard]] std::optional<Body> evaluated(const EvaluationContext &context) const override;
    void serialize(std::ostream &stream, const std::string &prefix) const override;

private:
    Visibility visibility;
    Symbol name;
    Expression value;
};


#endif // ASSIGNMENT_BODY_H
