//
// Created by Dieter Baron on 24.01.23.
//

#ifndef OBJECT_H
#define OBJECT_H

#include <vector>

#include "MemoryMap.h"
#include "ExpressionList.h"

class ObjectFile;

class Object {
public:
    enum Visibility {
        NONE,
        LOCAL,
        GLOBAL
    };

    Object(const ObjectFile* owner, symbol_t section, Visibility visibility, Token name): owner(owner), section(section), visibility(visibility), name(name) {}
    Object(const ObjectFile* owner, const Object* object);

    [[nodiscard]] bool is_reservation() const {return data.empty();}
    [[nodiscard]] bool empty() const {return is_reservation() && size == 0;}
    [[nodiscard]] bool has_address() const {return address.has_value();}

    void serialize(std::ostream& stream) const;

    void append(const ExpressionList& list);
    void append(const std::shared_ptr<Expression>& expression);

    const ObjectFile* owner;
    symbol_t section;
    Visibility visibility;
    Token name;
    uint64_t alignment = 0;
    uint64_t size = 0;
    std::optional<uint64_t> bank;
    std::optional<uint64_t> address;

    ExpressionList data;
};

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Object>& node);
std::ostream& operator<<(std::ostream& stream, const Object& node);
std::ostream& operator<<(std::ostream& stream, Object::Visibility visibility);


#endif // OBJECT_H
