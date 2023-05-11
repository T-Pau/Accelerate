//
// Created by Dieter Baron on 08.05.23.
//

#include "Body.h"
#include "BodyBlock.h"

void Body::append(const std::shared_ptr<BodyElement> &element) {
    if (element->empty()) {
        return;
    }
    if (!elements) {
        elements =  element;
    }
    else {
        auto new_body = elements->append_sub(elements, element);
        if (new_body) {
            elements = new_body;
        }
        else {
            elements = std::make_shared<BodyBlock>(std::vector<std::shared_ptr<BodyElement>>({elements, element}));
        }
    }
}


void Body::collect_objects(std::unordered_set<Object*> &objects) const {
    if (elements) {
        elements->collect_objects(objects);
    }
}

bool Body::evaluate(const Environment &environment) {
    auto result = elements->evaluate(environment, 0, 0);
    if (result.element) {
        elements = result.element;
        return true;
    }
    else {
        return false;
    }
}

std::shared_ptr<BodyElement> Body::back() const {
    auto block = std::dynamic_pointer_cast<BodyBlock>(elements);

    if (block) {
        return block->back();
    }
    else {
        return elements;
    }
}
