/*
Body.cc --

Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Body.h"

#include "AssignmentBody.h"
#include "BlockBody.h"
#include "DataBody.h"
#include "EmptyBody.h"
#include "ErrorBody.h"
#include "IfBody.h"
#include "LabelBody.h"
#include "MacroBody.h"
#include "MemoryBody.h"
#include "ScopeBody.h"

Body::Body(): element(std::make_shared<EmptyBody>()) {}

Body::Body(const std::shared_ptr<BodyElement> &new_element) {
    if (new_element) {
        element = new_element;
    }
    else {
        element = std::make_shared<EmptyBody>();
    }
}


Body::Body(Visibility visibility, Symbol name, Expression value) {
    element = std::make_shared<AssignmentBody>(visibility, name, std::move(value));
}


Body::Body(const std::vector<Body>& elements) {
    *this = BlockBody::create(elements);
}

Body::Body(std::vector<DataBodyElement> elements): element(std::make_shared<DataBody>(std::move(elements))) {}

Body::Body(Location location, std::string message): element(std::make_shared<ErrorBody>(location, std::move(message))) {}
Body::Body(const std::vector<IfBodyClause>& clauses) {
    *this = IfBody::create(clauses);
}

Body::Body(Symbol name): element(std::make_shared<LabelBody>(name)) {}
Body::Body(Symbol name, SizeRange offset, bool added_to_environment, size_t unnamed_index): element(std::make_shared<LabelBody>(name, offset, added_to_environment, unnamed_index)) {}

Body::Body(const Token& name, std::vector<Expression> arguments, const Macro* macro): element(std::make_shared<MacroBody>(name, std::move(arguments), macro)) {}

Body::Body(Expression bank, Expression start_address, Expression end_address): element(std::make_shared<MemoryBody>(std::move(bank), std::move(start_address), std::move(end_address))) {}

void Body::append(const Body& new_element) {
    if (new_element.empty()) {
        return;
    }
    if (element->empty()) {
        element = new_element.element;
    }
    else {
        if (const auto new_body = append_sub(new_element)) {
            element = new_body->element;
        }
        else {
            element = std::make_shared<BlockBody>(std::vector<Body>({*this, new_element}));
        }
    }
}

bool Body::evaluate(const EvaluationContext& context) {
    if (const auto new_elements = element->evaluated(context)) {
        element = new_elements->element;
        return true;
    }
    else {
        return false;
    }
}

std::optional<Body> Body::back() const {
    if (const auto block = as_block()) {
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

LabelBody* Body::as_label() const { return std::dynamic_pointer_cast<LabelBody>(element).get(); }

ScopeBody* Body::as_scope() const {return std::dynamic_pointer_cast<ScopeBody>(element).get();}

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

Body Body::scoped(std::shared_ptr<Environment> inner_environment) const {
    if (is_scope() && !inner_environment) {
        return *this;
    }
    else {
        return ScopeBody::create(*this, inner_environment);
    }
}

std::ostream& operator<<(std::ostream& stream, const Body& body) {
    body.serialize(stream, "    ");
    return stream;
}
