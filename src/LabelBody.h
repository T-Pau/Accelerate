//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_BODY_H
#define LABEL_BODY_H

#include "Body.h"
#include "Label.h"
#include "Value.h"
#include "Exception.h"
#include "SizeRange.h"

class LabelBody: public BodyElement {
public:
    explicit LabelBody(std::shared_ptr<Label> label): label(std::move(label)) {}

    void encode(std::string &bytes, const Memory* memory) const override {}
    [[nodiscard]] bool empty() const override {return label->offset.minimum == label->offset.maximum;}
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone label");}
    [[nodiscard]] std::optional<Body> evaluated(const EvaluationContext& context) const override;

    void serialize(std::ostream &stream, const std::string& prefix) const override;

    std::shared_ptr<Label> label;
};


#endif // LABEL_BODY_H
