//
// Created by Dieter Baron on 31.05.23.
//

#ifndef MACRO_BODY_H
#define MACRO_BODY_H

#include "BodyElement.h"
#include "Exception.h"
#include "ParseException.h"

class MacroBody: public BodyElement {
  public:
    MacroBody(Token name, std::vector<Expression> arguments): BodyElement(SizeRange(0,{})), name(name), arguments(std::move(arguments)) {}

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone MacroBody");}
    [[nodiscard]] bool empty() const override {return false;}
    void encode(std::string &bytes, const Memory *memory) const override {throw ParseException(name, "can't encode unexpanded macro call");}
    [[nodiscard]] std::optional<Body> evaluated(const EvaluationContext &context) const override;
    void serialize(std::ostream &stream, const std::string &prefix) const override;

    Token name;
    std::vector<Expression> arguments;
};


#endif // MACRO_BODY_H
