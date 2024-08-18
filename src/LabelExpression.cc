/*
LabelExpression.cc --

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

#include "LabelExpression.h"

#include "Entity.h"
#include "Expression.h"
#include "ObjectExpression.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "VariableExpression.h"
#include <complex>

LabelExpression::LabelExpression(const Location& location, const Entity* object, Symbol label_name, const SizeRange& offset) : BaseExpression(location), label_type(LabelExpressionType::NAMED), object_name(object ? object->name : Symbol()), label_name(label_name), object(object), offset(offset) {}

Expression LabelExpression::create(const Location& location, const std::vector<Expression>& arguments) {
    if (arguments.size() == 1) {
        if (arguments[0].has_value()) {
            auto& value = arguments[0];
            if (value.value()->is_unsigned()) {
                return create(location, nullptr, Symbol(), SizeRange(arguments[0].value()->unsigned_value()));
            }
            else {
                throw ParseException(location, "invalid arguments for .label_offset()");
            }
        }
        if (auto label_name = arguments[0].as_variable()) {
            if (label_name->variable() == Token::colon_minus.as_symbol()) {
                return create(location, PREVIOUS_UNNAMED);
            }
            else if (label_name->variable() == Token::colon_plus.as_symbol()) {
                return create(location, NEXT_UNNAMED);
            }
            else {
                throw ParseException(location, "invalid arguments for .label_offset()");
            }
        }
        else {
            throw ParseException(location, "invalid arguments for .label_offset()");
        }
    }
    else if (arguments.size() == 2) {
        auto object_name = arguments[0].as_variable();
        auto label_name = arguments[1].as_variable();
        if (object_name && label_name) {
            return Expression(std::make_shared<LabelExpression>(location, object_name->variable(),                label_name->variable()));
        }
        else {
            throw ParseException(location, "invalid arguments for .label_offset()");
        }
    }
    else {
        throw ParseException(location, "invalid number of arguments for .label_offset()");
    }

}

Expression LabelExpression::create(const Location& location, const Entity* object, Symbol label_name, const SizeRange& offset, const SizeRange& scope_offset, bool keep) {
    if (!keep && offset.has_size() && scope_offset.has_size()) {
        return {location, Expression({}, *offset.size()), Expression::ADD, Expression({}, *scope_offset.size())};
    }
    else {
        return Expression(std::make_shared<LabelExpression>(location, object, label_name, offset));
    }
}

Expression LabelExpression::create(const Location& location, LabelExpressionType type, size_t unnamed_index, SizeRange offset) {
    if (offset.size()) {
        return Expression(location, *offset.size());
    }
    else {
        return Expression(std::make_shared<LabelExpression>(location, type, unnamed_index, offset));
    }
}

std::optional<Expression> LabelExpression::evaluated(const EvaluationContext& context) const {
    const Entity* new_object = object;
    auto new_offset = offset;
    auto new_label_name = label_name;

    if (label_type == PREVIOUS_UNNAMED || label_type == NEXT_UNNAMED) {
        auto unnamed_label = context.result.next_unnamed_label;
        if (label_type == PREVIOUS_UNNAMED) {
            if (unnamed_label == 0) {
                throw ParseException(location, "no previous unnamed label");
            }
            unnamed_label -= 1;
        }
        new_label_name = LabelBody::unnamed_label_name(unnamed_label);
    }

    if (!object_name.empty() || !context.labels_are_offset) {
        // TODO: review this section for new label resolution
        if (!object) {
            if (context.entity && context.entity->is_object()) {
                if (object_name.empty() || object_name == context.entity->name) {
                    new_object = context.entity;
                }
                else {
                    new_object = context.entity->owner->object(object_name); // TODO: search in included libraries too
                }
            }
            else if (!object_name.empty()) {
                auto value = context.environment->get_variable(object_name);
                if (value && value->is_object()) {
                    context.result.used_objects.insert(value->as_object()->object);
                    new_object = value->as_object()->object;
                }
                // TODO: handle unresolved label
            }
        }
    }

    if (new_object) {
        // TODO: handle scope
        if (auto found_offset = new_object->environment->get_label(label_name)) {
            new_offset = *found_offset;
        }
    }
    else if (object_name.empty()) {
        if (auto found_offset = context.environment->get_label(label_name)) {
            new_offset = *found_offset;
        }
    }

    if (new_object != object || new_label_name != label_name || new_offset != offset || (new_offset.has_size() && context.label_offset.has_size())) {
        return Expression(location, new_object, new_label_name, new_offset, context.label_offset, context.keep_label_offsets);
    }

    return {};
}

void LabelExpression::serialize_sub(std::ostream& stream) const {
    stream << ".label_offset(";
    if (offset.has_size()) {
        auto value = *offset.size();

        stream << "$" << std::setfill('0') << std::setw(static_cast<int>(Int::minimum_byte_size(value) * 2)) << std::hex << value << std::dec;
    }
    else {
        switch (label_type) {
            case NAMED:
                if (!object_name.empty()) {
                    stream << object_name << ", ";
                }
                stream << label_name;
                break;

            case NEXT_UNNAMED:
                stream << ":+";
                break;

            case PREVIOUS_UNNAMED:
                stream << ":-";
        }
    }
    stream << ")";
}
