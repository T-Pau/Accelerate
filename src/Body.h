//
// Created by Dieter Baron on 08.05.23.
//

#ifndef BODY_H
#define BODY_H

#include "BodyElement.h"
#include "Memory.h"

class Body {
public:
    Body() = default;
    explicit Body(std::shared_ptr<BodyElement> element): elements(std::move(element)) {}

    void append(const std::shared_ptr<BodyElement>& element);
    [[nodiscard]] std::shared_ptr<BodyElement> back() const;
    [[nodiscard]] bool empty() const {return !elements || elements->empty();}
    void encode(std::string& bytes, const Memory* memory = nullptr) const {if (elements) {elements->encode(bytes, memory);}}
    bool evaluate(const Environment& environment);
    [[nodiscard]] uint64_t maximum_size() const {return elements ? elements->maximum_size() : 0;}
    [[nodiscard]] uint64_t minimum_size() const {return elements ? elements->minimum_size() : 0;}
    void serialize(std::ostream& stream, const std::string& prefix = "") const {if (elements) {elements->serialize(stream, prefix);}}
    [[nodiscard]] std::optional<uint64_t> size() const {return elements ? elements->size() : 0;}

    std::shared_ptr<BodyElement> elements;
};


#endif // BODY_H
