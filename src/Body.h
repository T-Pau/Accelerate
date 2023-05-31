//
// Created by Dieter Baron on 08.05.23.
//

#ifndef BODY_H
#define BODY_H

#include "BodyElement.h"
#include "EvaluationContext.h"
#include "Label.h"
#include "Location.h"
#include "SizeRange.h"
#include "Visibility.h"

class BlockBody;
class DataBody;
class DataBodyElement;
class ErrorBody;
class IfBodyClause;
class LabelBody;

class Body {
public:

    Body();
    explicit Body(const std::shared_ptr<BodyElement>& element);
    // Assignment
    Body(Visibility visibility, Symbol name, Expression value);
    // Block
    explicit Body(const std::vector<Body>& elements);
    // Data
    explicit Body(std::vector<DataBodyElement> elements);
    // Error
    Body(Location location, std::string message);
    // If
    explicit Body(const std::vector<IfBodyClause>& clauses);
    // Label
    explicit Body(std::shared_ptr<Label> label);
    // Macro
    Body(Token name, std::vector<Expression> arguments);
    // Memory
    Body(Expression bank, Expression start_address, Expression end_address);

    [[nodiscard]] BlockBody* as_block() const;
    [[nodiscard]] DataBody* as_data() const;
    [[nodiscard]] ErrorBody* as_error() const;
    [[nodiscard]] LabelBody* as_label() const;
    void append(const Body& element);
    std::optional<Body> append_sub(Body element);
    [[nodiscard]] std::optional<Body> back() const;
    void collect_objects(std::unordered_set<Object*>& objects) const {element->collect_objects(objects);}
    [[nodiscard]] bool empty() const {return element->empty();}
    [[nodiscard]] Body make_unique() const;
    void encode(std::string& bytes, const Memory* memory = nullptr) const {element->encode(bytes, memory);}
    bool evaluate(Symbol object_name, ObjectFile* object_file, std::shared_ptr<Environment> environment, bool conditional = false, SizeRange offset = SizeRange()) {return evaluate(EvaluationContext(object_name, object_file, environment, conditional, offset));}
    bool evaluate(const EvaluationContext& context);
    [[nodiscard]] std::optional<Body> evaluated (const EvaluationContext& context) const;
    [[nodiscard]] bool is_block() const {return as_block() != nullptr;}
    [[nodiscard]] bool is_data() const {return as_data() != nullptr;}
    [[nodiscard]] bool is_error() const {return as_error() != nullptr;}
    [[nodiscard]] bool is_label() const {return as_label() != nullptr;}
    void serialize(std::ostream& stream, const std::string& prefix = "") const {element->serialize(stream, prefix);}
    [[nodiscard]] std::optional<uint64_t> size() const {return element->size();}
    [[nodiscard]] SizeRange size_range() const {return element->size_range();}
    [[nodiscard]] long use_count() const {return element.use_count();}

private:
    std::shared_ptr<BodyElement> element;


};

#endif // BODY_H
