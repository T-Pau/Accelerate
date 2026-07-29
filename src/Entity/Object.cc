/*
Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "Entity/Object.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "Body/BodyElement.h"
#include "ExpressionParser.h"
#include "Scope.h"
#include "SequenceTokenizer.h"
#include "StructuredBody.h"
#include "StructuredDictionary.h"
#include "StructuredScalar.h"

using namespace tpau::cpp_kernal;

#define ADDRESS "address"
#define ALIGNMENT "alignment"
#define BODY "body"
#define RESERVE "reserve"
#define SECTION "section"
#define USES "uses"

const Token Object::token_address{Token::NAME, ADDRESS};
const Token Object::token_alignment{Token::NAME, ALIGNMENT};
const Token Object::token_body{Token::NAME, BODY};
const Token Object::token_reserve{Token::NAME, RESERVE};
const Token Object::token_section{Token::NAME, SECTION};
const Token Object::token_uses{Token::NAME, USES};

Object::Object(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition) : ScopeEntity(location, name, parent_scope, definition) {
    auto parameters = definition->as_dictionary();

    if (auto address_value = parameters->get_optional(token_address)) {
        auto tokenizer = SequenceTokenizer(address_value->location, address_value->as_scalar()->tokens);
        address = Address(tokenizer);
        if (!tokenizer.ended()) {
            throw LocationException(tokenizer.current_location(), "expected newline");
        }
    }

    if (auto alignment_value = parameters->get_optional(token_alignment)) {
        auto alignment_token = alignment_value->as_singular_scalar()->token();
        if (!alignment_token.is_unsigned()) {
            throw LocationException(alignment_token.location, "unsigned integer expected");
        }
        alignment = alignment_token.as_unsigned();
    }

    if (auto uses_value = parameters->get_optional(token_uses)) {
        for (const auto& object_name : uses_value->as_scalar()->tokens) {
            uses(object_name.as_symbol());
        }
    }

    auto body_value = parameters->get_optional(token_body);
    auto reserve_value = parameters->get_optional(token_reserve);
    if ((body_value && reserve_value) || (!body_value && !reserve_value)) {
        throw LocationException(parameters->location, "object must contain exactly one of reserve and body");
    }
    if (reserve_value) {
        if (!reserve_value->is_scalar()) {
            throw LocationException(reserve_value->location, ".reserve must be scalar");
        }
        auto tokenizer = SequenceTokenizer{reserve_value->as_scalar()->tokens};
        reservation_expression = ExpressionParser{tokenizer}.parse();
    }
    else {
        body = body_value->as_body()->body;
        evaluate();
    }

    // TODO
    // section = owner->target->map.section((*parameters)[token_section]->as_singular_scalar()->token().as_symbol());
}

std::ostream& operator<<(std::ostream& stream, const Object& object) {
    object.serialize(stream);
    return stream;
}

void Object::serialize(std::ostream& stream) const {
    stream << ".object " << name << " {" << std::endl;
    serialize_entity(stream);
    if (address) {
        stream << "    " ADDRESS ": " << *address << std::endl;
    }
    else if (alignment > 0) {
        stream << "    " ALIGNMENT ": " << alignment << std::endl;
    }
    stream << "    " SECTION ": " << section->name << std::endl;
    if (!explicitly_used_objects.empty()) {
        stream << "    " USES ":";
        for (const auto& name : explicitly_used_objects) {
            stream << " " << name;
        }
        stream << std::endl;
    }
    if (is_reservation()) {
        stream << "    " RESERVE ": " << *reservation_expression << std::endl;
    }
    else {
        stream << "    " BODY " <" << std::endl;
        // TODO: avoid code duplication with ScopedBody
        auto constants = scope()->get_constants();
        auto sorted_constants = sorted(constants.begin(), constants.end(), [](const auto& a, const auto& b) { return a->get_name() < b->get_name(); });
        for (const auto& constant : sorted_constants) {
            stream << "        " << constant->get_name() << " = " << constant->value << std::endl;
        }

        body.serialize(stream, "        ");
        stream << "    >" << std::endl;
    }
    stream << "}" << std::endl;
}

SizeRange Object::size_range() const {
    if (is_reservation()) {
        auto minimum_value = reservation_expression->minimum_value();
        auto maximum_value = reservation_expression->maximum_value();
        if ((minimum_value && !minimum_value->is_unsigned()) || (maximum_value && !maximum_value->is_unsigned())) {
            throw LocationException(reservation_expression->location(), "reservation must be unsigned");
        }
        return {minimum_value ? minimum_value->unsigned_value() : 0, maximum_value ? maximum_value->unsigned_value() : std::optional<uint64_t>{}};
    }
    else {
        return body.size_range();
    }
}

void Object::evaluate_implementation(EvaluationContext& context) {
    if (address) {
        address->evaluate(context);
    }
    if (is_reservation()) {
        reservation_expression->evaluate(context);
    }
    else {
        // We do this twice for now to propagate label offsets.
        // TODO: Propagate label offsets in a more efficient way that guarantees correctness.

        for (auto& constant : constants) {
            constant->evaluate();
        }
        body.evaluate(context);
        for (auto& constant : constants) {
            constant->evaluate();
        }
        body.evaluate(context);
    }
#ifdef TRACE_TRANSLATION
    std::cerr << *this;
#endif
}

bool Object::operator<(const Object& other) const {
    if (address && other.address) {
        if (*address != *other.address) {
            return *address < *other.address;
        }
    }
    else if (address) {
        return true;
    }
    else if (other.address) {
        return false;
    }

    if (*section < *other.section) {
        return true;
    }
    else if (*other.section < *section) {
        return false;
    }

    if (size_range().size() && other.size_range().size() && size_range().size() != other.size_range().size()) {
        return size_range().size() > other.size_range().size();
    }

    return name < other.name;
}

void Object::pin(Expression expression) {
    if (address) {
        throw LocationException(expression.location(), "{} already has an address", name);
    }
    address = Address(expression);
}

std::optional<uint64_t> Object::maximum_address() const {
    auto maximum_size = size_range().maximum;
    auto maximum = section->maximum_address() - (maximum_size ? *maximum_size : 0);
    if (address) {
        auto address_maximum = address->address_maximum();
        if (address_maximum) {
            maximum = std::min(maximum, *address_maximum);
        }
    }
    return maximum;
}

std::optional<uint64_t> Object::minimum_address() const {
    auto minimum = section->minimum_address();
    if (address) {
        auto address_minimum = address->address_minimum();
        if (address_minimum) {
            minimum = std::max(minimum, *address_minimum);
        }
    }
    return minimum;
}

void Object::traverse(std::function<void(Entity&)> entity_callback, std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) {
    ScopeEntity::traverse(entity_callback, body_callback, expression_callback);
    if (address) {
        address->traverse(expression_callback);
    }
    if (reservation_expression) {
        expression_callback(*reservation_expression);
    }
    else {
        for (auto& constant : constants) {
            entity_callback(*constant);
        }
        body_callback(body);
    }
}

void Object::enter_names() {
    TRACE_BEGIN("entering names", "{}", name);
    body.enter_names(scope().get(), this);
    TRACE_END("entering names", "{}", name);
}

void Object::resolve_implementation() {
    if (address) {
        address->resolve(scope().get(), this);
    }
    if (reservation_expression) {
        reservation_expression->resolve(scope().get(), this);
    }
    else {
        resolve_constants();
        body.resolve(scope().get(), this);
    }
}
