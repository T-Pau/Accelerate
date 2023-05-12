//
// Created by Dieter Baron on 10.05.23.
//

#ifndef MEMORY_BODY_H
#define MEMORY_BODY_H

#include "Body.h"
#include "Exception.h"

class MemoryBody: public BodyElement {
public:
    MemoryBody(Expression bank, Expression start_address, Expression end_address);

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<MemoryBody>(bank, start_address, end_address);}
    [[nodiscard]] bool empty() const override {return size().value_or(1) == 0;}
    void encode(std::string &bytes, const Memory* memory) const override;
    [[nodiscard]] std::optional<Body> evaluated(const Environment &environment, const SizeRange& offset) const override;
    void serialize(std::ostream &stream, const std::string &prefix) const override;

    Expression bank;
    Expression start_address;
    Expression end_address;
};


#endif // MEMORY_BODY_H
