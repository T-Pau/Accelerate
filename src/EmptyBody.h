//
// Created by Dieter Baron on 11.05.23.
//

#ifndef EMPTY_BODY_H
#define EMPTY_BODY_H

#include "Body.h"

class EmptyBody: public BodyElement {
public:
    EmptyBody() = default;
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<EmptyBody>();}
    [[nodiscard]] bool empty() const override {return true;}
    void encode(std::string &bytes, const Memory *memory) const override {}
    [[nodiscard]] std::optional<Body> evaluated(const Environment &environment, bool top_level, const SizeRange& offset) const override {return {};}
    void serialize(std::ostream &stream, const std::string &prefix) const override {}
};

#endif // EMPTY_BODY_H
