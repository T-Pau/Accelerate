//
// Created by Dieter Baron on 24.01.23.
//

#ifndef OBJECT_H
#define OBJECT_H

#include <vector>

#include "MemoryMap.h"
#include "ExpressionList.h"

class Object {
public:
    enum Visibility {
        NONE,
        LOCAL,
        GLOBAL
    };

    Object(symbol_t section, Visibility visibility, Token name): section(section), visibility(visibility), name(name) {}

    [[nodiscard]] bool is_reservation() const {return data.empty();}
    [[nodiscard]] bool empty() const {return is_reservation() && size == 0;}

    void serialize(std::ostream& stream) const;

    void append(const ExpressionList& list);
    void append(const std::shared_ptr<ExpressionNode>& expression);

    symbol_t section;
    Visibility visibility;
    Token name;
    uint64_t alignment = 0;
    uint64_t size = 0;

    ExpressionList data;
};

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Object>& node);
std::ostream& operator<<(std::ostream& stream, const Object& node);
std::ostream& operator<<(std::ostream& stream, Object::Visibility visibility);


#endif // OBJECT_H
