/*
Object.cc --

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

#include "Object.h"

#include "BodyElement.h"
#include "ExpressionParser.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

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


Object::Object(ObjectFile* owner, const Token& name, const std::shared_ptr<ParsedValue>& definition): Entity(owner, name, definition) {
    auto parameters = definition->as_dictionary();

    if (auto address_value = parameters->get_optional(token_address)) {
        auto tokenizer = SequenceTokenizer(address_value->location, address_value->as_scalar()->tokens);
        address = Address(tokenizer);
        if (!tokenizer.ended()) {
            throw ParseException(tokenizer.current_location(), "expected newline");
        }
    }

    if (auto alignment_value = parameters->get_optional(token_alignment)) {
        auto alignment_token = alignment_value->as_singular_scalar()->token();
        if (!alignment_token.is_unsigned()) {
            throw ParseException(alignment_token, "unsigned integer expected");
        }
        alignment = alignment_token.as_unsigned();
    }

    if (auto uses_value = parameters->get_optional(token_uses)) {
        for (const auto& object_name: uses_value->as_scalar()->tokens) {
            uses(object_name.as_symbol());
        }
    }

    auto body_value = parameters->get_optional(token_body);
    auto reserve_value = parameters->get_optional(token_reserve);
    if ((body_value && reserve_value) || (!body_value && !reserve_value)) {
        throw ParseException(parameters->location, "object must contain exactly one of reserve and body");
    }
    if (reserve_value) {
        if (!reserve_value->is_scalar()) {
            throw ParseException(reserve_value->location, ".reserve must be scalar");
        }
        auto tokenizer = SequenceTokenizer{reserve_value->as_scalar()->tokens};
        reservation_expression = ExpressionParser{tokenizer}.parse();
    }
    else {
        body = body_value->as_body()->body;
        evaluate();
    }

    section = owner->target->map.section((*parameters)[token_section]->as_singular_scalar()->token().as_symbol());
}


Object::Object(ObjectFile *owner, const MemoryMap::Section *section, Visibility visibility, bool default_only, const Token& name): Entity(owner, name, visibility, default_only), section(section) {}


std::ostream& operator<< (std::ostream& stream, const Object& object) {
    object.serialize(stream);
    return stream;
}


void Object::serialize(std::ostream &stream) const {
    stream << ".object " << name.as_string() << " {" << std::endl;
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
        for (const auto& name: explicitly_used_objects) {
            stream << " " << name;
        }
        stream << std::endl;
    }
    if (is_reservation()) {
        stream << "    " RESERVE ": " << *reservation_expression << std::endl;
    }
    else {
        stream << "    " BODY " <" << std::endl;
        body.serialize(stream, "        ");
        stream << "    >" << std::endl;
    }
    stream << "}" << std::endl;
}


SizeRange Object::size_range() const {
    if (is_reservation()) {
        auto minimum_value = reservation_expression->minimum_value();
        auto maximum_value = reservation_expression->maximum_value();
        if ((minimum_value && !minimum_value->is_unsigned()) || maximum_value && !maximum_value->is_unsigned()) {
            throw ParseException(reservation_expression->location(), "reservation must be unsigned");
        }
        return {minimum_value ? minimum_value->unsigned_value() : 0, maximum_value ? maximum_value->unsigned_value() : std::optional<uint64_t>{}};
    }
    else {
        return body.size_range();
    }
}


void Object::evaluate_inner(EvaluationContext& context) {
    if (is_reservation()) {
        reservation_expression->evaluate(context);
    }
    else {
        body.evaluate(context);
    }
}


bool Object::operator<(const Object& other) const {
    if (address) {
        if (other.address) {
            if (*address != *other.address) {
                return *address < *other.address;
            }
        }
        else {
            return true;
        }
    }
    else {
        if (other.address) {
            return false;
        }
        if (*section < *other.section) {
            return true;
        }
        else if (*other.section < *section) {
            return false;
        }
    }

    if (size_range().size() && other.size_range().size() && size_range().size() != other.size_range().size()) {
        return size_range().size() > other.size_range().size();
    }

    return name.as_symbol() < other.name.as_symbol();
}
