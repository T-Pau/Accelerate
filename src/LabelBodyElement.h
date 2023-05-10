//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_BODY_ELEMENT_H
#define LABEL_BODY_ELEMENT_H

#include "BodyElement.h"
#include "Value.h"
#include "Exception.h"
#include "SizeRange.h"

class LabelBodyElement: public BodyElement {
public:
    LabelBodyElement(Symbol name, SizeRange offset): name(name), offset(offset) {}

    void encode(std::string &bytes) const override {}
    [[nodiscard]] bool empty() const override {return offset.minimum == offset.maximum;}
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone label");}
    [[nodiscard]] EvaluationResult evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const override;
    [[nodiscard]] uint64_t minimum_size() const override {return 0;}
    [[nodiscard]] uint64_t maximum_size() const override {return 0;}
    [[nodiscard]] std::optional<uint64_t> size() const override {return 0;}

    [[nodiscard]] Value minimum_value() const {return Value(offset.minimum);}
    [[nodiscard]] std::optional<Value> maximum_value() const;
    [[nodiscard]] std::optional<Value> value() const;

    void serialize(std::ostream &stream, const std::string& prefix) const override;

    const Symbol name;

private:
    mutable SizeRange offset;
};


#endif // LABEL_BODY_ELEMENT_H
