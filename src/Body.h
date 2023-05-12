//
// Created by Dieter Baron on 08.05.23.
//

#ifndef BODY_H
#define BODY_H

#include "BodyElement.h"
#include "Label.h"
#include "SizeRange.h"

class BlockBody;
class DataBody;
class DataBodyElement;
class IfBodyClause;
class LabelBody;

class Body {
public:

    Body();
    explicit Body(const std::shared_ptr<BodyElement>& element);
    // Block
    explicit Body(const std::vector<Body>& elements);
    // Data
    explicit Body(std::vector<DataBodyElement> elements);
    // If
    explicit Body(const std::vector<IfBodyClause>& clauses);
    // Label
    explicit Body(std::shared_ptr<Label> label);
    // Memory
    Body(Expression bank, Expression start_address, Expression end_address);

    [[nodiscard]] BlockBody* as_block() const;
    [[nodiscard]] DataBody* as_data() const;
    [[nodiscard]] LabelBody* as_label() const;
    void append(const Body& element);
    std::optional<Body> append_sub(Body element);
    [[nodiscard]] std::optional<Body> back() const;
    void collect_objects(std::unordered_set<Object*>& objects) const {element->collect_objects(objects);}
    [[nodiscard]] bool empty() const {return element->empty();}
    [[nodiscard]] Body make_unique() const;
    void encode(std::string& bytes, const Memory* memory = nullptr) const {element->encode(bytes, memory);}
    bool evaluate(const Environment& environment, const SizeRange& offset = SizeRange(0));
    [[nodiscard]] std::optional<Body> evaluated (const Environment& environment, const SizeRange& offset) const;
    [[nodiscard]] bool is_block() const {return as_block() != nullptr;}
    [[nodiscard]] bool is_data() const {return as_data() != nullptr;}
    [[nodiscard]] bool is_label() const {return as_label() != nullptr;}
    void serialize(std::ostream& stream, const std::string& prefix = "") const {element->serialize(stream, prefix);}
    [[nodiscard]] std::optional<uint64_t> size() const {return element->size();}
    [[nodiscard]] SizeRange size_range() const {return element->size_range();}
    [[nodiscard]] long use_count() const {return element.use_count();}

private:
    std::shared_ptr<BodyElement> element;
};

#endif // BODY_H
