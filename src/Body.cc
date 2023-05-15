//
// Created by Dieter Baron on 08.05.23.
//

#include "Body.h"

#include <utility>

#include "BlockBody.h"
#include "DataBody.h"
#include "EmptyBody.h"
#include "ErrorBody.h"
#include "IfBody.h"
#include "LabelBody.h"
#include "MemoryBody.h"

Body::Body(): element(std::make_shared<EmptyBody>()) {}

Body::Body(const std::shared_ptr<BodyElement> &new_element) {
    if (new_element) {
        element = new_element;
    }
    else {
        element = std::make_shared<EmptyBody>();
    }
}

Body::Body(const std::vector<Body>& elements) {
    *this = BlockBody::create(elements);
}

Body::Body(std::vector<DataBodyElement> elements): element(std::make_shared<DataBody>(std::move(elements))) {}

Body::Body(Location location, std::string message): element(std::make_shared<ErrorBody>(location, std::move(message))) {}
Body::Body(const std::vector<IfBodyClause>& clauses) {
    *this = IfBody::create(clauses);
}

Body::Body(std::shared_ptr<Label> label): element(std::make_shared<LabelBody>(std::move(label))) {}

Body::Body(Expression bank, Expression start_address, Expression end_address): element(std::make_shared<MemoryBody>(std::move(bank), std::move(start_address), std::move(end_address))) {}

void Body::append(const Body& new_element) {
    if (new_element.empty()) {
        return;
    }
    if (element->empty()) {
        element = new_element.element;
    }
    else {
        auto new_body = append_sub(new_element);
        if (new_body) {
            element = new_body->element;
        }
        else {
            element = std::make_shared<BlockBody>(std::vector<Body>({*this, new_element}));
        }
    }
}

bool Body::evaluate(const EvaluationContext& context) {
    auto new_elements = element->evaluated(context);
    if (new_elements) {
        element = new_elements->element;
        return true;
    }
    else {
        return false;
    }
}

std::optional<Body> Body::back() const {
    auto block = as_block();

    if (block) {
        return block->back();
    }
    else if (!empty()) {
        return *this;
    }
    else {
        return {};
    }
}

std::optional<Body> Body::evaluated(const EvaluationContext& context) const {
    return element->evaluated(context);
}

BlockBody *Body::as_block() const {
    return std::dynamic_pointer_cast<BlockBody>(element).get();
}

DataBody *Body::as_data() const {
    return std::dynamic_pointer_cast<DataBody>(element).get();
}

ErrorBody *Body::as_error() const {
    return std::dynamic_pointer_cast<ErrorBody>(element).get();
}

LabelBody *Body::as_label() const {
    return std::dynamic_pointer_cast<LabelBody>(element).get();
}

std::optional<Body> Body::append_sub(Body new_element) {
    return element->append_sub(*this, std::move(new_element));
}

Body Body::make_unique() const {
    if (element.use_count() == 1) {
        return *this;
    }
    else {
        return Body(element->clone());
    }
}
