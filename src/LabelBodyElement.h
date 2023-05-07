//
// Created by Dieter Baron on 03.05.23.
//

#ifndef LABEL_BODY_ELEMENT_H
#define LABEL_BODY_ELEMENT_H

#include "BodyElement.h"
#include "Value.h"
#include "Exception.h"

class LabelBodyElement: public BodyElement {
public:
    LabelBodyElement(Symbol name, uint64_t minimum_offset, uint64_t maximum_offset): name(name), minimum_offset(minimum_offset), maximum_offset(maximum_offset) {}

    void encode(std::string &bytes) const override {}
    [[nodiscard]] bool empty() const override {return minimum_offset == maximum_offset;}
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone label");}
    [[nodiscard]] EvaluationResult evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const override;
    [[nodiscard]] uint64_t minimum_size() const override {return 0;}
    [[nodiscard]] uint64_t maximum_size() const override {return 0;}
    [[nodiscard]] std::optional<uint64_t> size() const override {return 0;}

    [[nodiscard]] Value minimum_value() const {return Value(minimum_offset);}
    [[nodiscard]] Value maximum_value() const {return Value(maximum_offset);}
    [[nodiscard]] std::optional<Value> value() const;

    void serialize(std::ostream &stream, const std::string& prefix = "") const override;

    const Symbol name;

private:
    mutable uint64_t minimum_offset = 0;
    mutable uint64_t maximum_offset = std::numeric_limits<uint64_t>::max();
};


#endif // LABEL_BODY_ELEMENT_H
