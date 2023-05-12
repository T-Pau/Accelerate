//
// Created by Dieter Baron on 08.05.23.
//

#include "Body.h"

#include <utility>

#include "BodyBlock.h"
#include "DataBodyElement.h"
#include "EmptyBodyElement.h"
#include "LabelBodyElement.h"

Body::Body(): elements(std::make_shared<EmptyBodyElement>()) {}

Body::Body(const std::shared_ptr<BodyElement> &element) {
    if (element) {
        elements = element;
    }
    else {
        elements = std::make_shared<EmptyBodyElement>();
    }
}

Body::Body(const std::shared_ptr<BodyElement> &element, SizeRange size_range) {
    if (element) {
        elements = element;
        elements->size_range_ = size_range;
    }
    else {
        elements = std::make_shared<EmptyBodyElement>();
    }
}


void Body::append(const Body& element) {
    if (element.empty()) {
        return;
    }
    if (!elements) {
        elements = element.elements;
    }
    else {
        auto new_body = append_sub(element);
        if (new_body) {
            elements = new_body->elements;
        }
        else {
            elements = std::make_shared<BodyBlock>(std::vector<Body>({*this, element}));
        }
    }
}

bool Body::evaluate(const Environment &environment, const SizeRange& offset) {
    auto new_elements = elements->evaluated(environment, offset);
    if (new_elements) {
        elements = new_elements->elements;
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

std::optional<Body> Body::evaluated(const Environment &environment, const SizeRange& offset) const {
    return elements->evaluated(environment, offset);
}

BodyBlock *Body::as_block() const {
    return std::dynamic_pointer_cast<BodyBlock>(elements).get();
}

DataBodyElement *Body::as_data() const {
    return std::dynamic_pointer_cast<DataBodyElement>(elements).get();
}

LabelBodyElement *Body::as_label() const {
    return std::dynamic_pointer_cast<LabelBodyElement>(elements).get();
}

std::optional<Body> Body::append_sub(Body element) {
    return elements->append_sub(*this, std::move(element));
}

Body Body::make_unique() const {
    if (elements.use_count() == 1) {
        return *this;
    }
    else {
        return Body(elements->clone());
    }
}
