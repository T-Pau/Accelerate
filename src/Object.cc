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
#include "ObjectFile.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

#define ADDRESS "address"
#define ALIGNMENT "alignment"
#define BODY "body"
#define RESERVE "reserve"
#define SECTION "section"

const Token Object::token_address{Token::NAME, ADDRESS};
const Token Object::token_alignment{Token::NAME, ALIGNMENT};
const Token Object::token_body{Token::NAME, BODY};
const Token Object::token_reserve{Token::NAME, RESERVE};
const Token Object::token_section{Token::NAME, SECTION};


Object::Object(ObjectFile* owner, Token name_, const std::shared_ptr<ParsedValue>& definition): Entity(owner, name_, definition) {
    auto parameters = definition->as_dictionary();

    auto address_value = parameters->get_optional(token_address);
    if (address_value) {
        auto tokenizer = SequenceTokenizer(address_value->location, address_value->as_scalar()->tokens);
        address = Address(tokenizer);
        if (!tokenizer.ended()) {
            throw ParseException(tokenizer.current_location(), "expected newline");
        }
    }

    auto alignment_value = parameters->get_optional(token_alignment);
    if (alignment_value) {
        auto alignment_token = alignment_value->as_singular_scalar()->token();
        if (!alignment_token.is_unsigned()) {
            throw ParseException(alignment_token, "unsigned integer expected");
        }
        alignment = alignment_token.as_unsigned();
    }

    auto body_value = parameters->get_optional(token_body);
    auto reserve_value = parameters->get_optional(token_reserve);
    if ((body_value && reserve_value) || (!body_value && !reserve_value)) {
        throw ParseException(parameters->location, "object must contain exactly one of reserve and body");
    }
    if (reserve_value) {
        auto reserve = reserve_value->as_singular_scalar()->token();
        if (!reserve.is_unsigned()) {
            throw ParseException(reserve, "unsigned integer expected");
        }
        reservation = reserve.as_unsigned();
    }
    else {
        body = body_value->as_body()->body;
        evaluate();
    }

    section = owner->target->map.section((*parameters)[token_section]->as_singular_scalar()->token().as_symbol());
}


Object::Object(ObjectFile *owner, const MemoryMap::Section *section, Visibility visibility, Token name): Entity(owner, name, visibility), section(section) {}


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
    if (!body.empty()) {
        stream << "    " BODY " <" << std::endl;
        body.serialize(stream, "        ");
        stream << "    >" << std::endl;
    }
    else {
        stream << "    " RESERVE ": " << reservation << std::endl;
    }
    stream << "}" << std::endl;
}


SizeRange Object::size_range() const {
    if (is_reservation()) {
        return SizeRange(reservation);
    }
    else {
        return body.size_range();
    }
}


void Object::evaluate() {
    EvaluationResult result;
    body.evaluate(EvaluationContext(result, this));
    process_result(result);
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
