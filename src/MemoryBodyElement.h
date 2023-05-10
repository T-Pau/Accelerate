//
// Created by Dieter Baron on 10.05.23.
//

#ifndef MEMORY_BODY_ELEMENT_H
#define MEMORY_BODY_ELEMENT_H

#include "BodyElement.h"
#include "Exception.h"

class MemoryBodyElement: public BodyElement {
public:
    MemoryBodyElement(Expression start_address, Expression end_address): start_address(std::move(start_address)), end_address(std::move(end_address)) {}

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<MemoryBodyElement>(start_address, end_address);}
    bool empty() const override {return size().value_or(1) == 0;}
    void encode(std::string &bytes) const override {throw Exception("can't encode memory body element");}
    [[nodiscard]] EvaluationResult evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const override;
    [[nodiscard]] uint64_t maximum_size() const override;
    [[nodiscard]] uint64_t minimum_size() const override;
    void serialize(std::ostream &stream, const std::string &prefix) const override;
    [[nodiscard]] std::optional<uint64_t> size() const override;

    Expression start_address;
    Expression end_address;
};


#endif // MEMORY_BODY_ELEMENT_H
