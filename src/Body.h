//
// Created by Dieter Baron on 08.05.23.
//

#ifndef BODY_H
#define BODY_H

#include "BodyElement.h"
#include "SizeRange.h"

class BodyBlock;
class DataBodyElement;
class LabelBodyElement;

class Body {
public:

    Body();
    explicit Body(const std::shared_ptr<BodyElement>& element);
    explicit Body(const std::shared_ptr<BodyElement>& element, SizeRange size_range);

    BodyBlock* as_block() const;
    DataBodyElement* as_data() const;
    LabelBodyElement* as_label() const;
    void append(const Body& element);
    std::optional<Body> append_sub(Body element);
    [[nodiscard]] std::optional<Body> back() const;
    void collect_objects(std::unordered_set<Object*>& objects) const {elements->collect_objects(objects);}
    [[nodiscard]] bool empty() const {return elements->empty();}
    Body make_unique() const;
    void encode(std::string& bytes, const Memory* memory = nullptr) const {elements->encode(bytes, memory);}
    bool evaluate(const Environment& environment, const SizeRange& offset = SizeRange(0));
    [[nodiscard]] std::optional<Body> evaluated (const Environment& environment, const SizeRange& offset) const;
    [[nodiscard]] bool is_block() const {return as_block() != nullptr;}
    [[nodiscard]] bool is_label() const {return as_label() != nullptr;}
    void serialize(std::ostream& stream, const std::string& prefix = "") const {elements->serialize(stream, prefix);}
    [[nodiscard]] std::optional<uint64_t> size() const {return elements->size();}
    [[nodiscard]] SizeRange size_range() const {return elements->size_range();}
    [[nodiscard]] long use_count() const {return elements.use_count();}

private:
    std::shared_ptr<BodyElement> elements;
};

#endif // BODY_H
