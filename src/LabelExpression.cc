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
#include "Object.h"
#include "ObjectExpression.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "VariableExpression.h"

LabelExpression::LabelExpression(Location location, const Entity* object, Symbol label_name, SizeRange offset): BaseExpression(location), type(LabelExpressionType::NAMED), object(object), object_name(object ? object->name.as_symbol() : Symbol()), label_name(label_name), offset(offset) {}

Expression LabelExpression::create(const std::vector<Expression>& arguments) {
    if (arguments.size() == 1) {
        auto label_name = arguments[0].as_variable();
        if (!label_name) {
            throw ParseException(arguments[0].location(), "invalid arguments for .label_offset()");
        }
        else if (label_name->variable() == Token::colon_minus.as_symbol()) {
            return Expression(std::make_shared<LabelExpression>(label_name->location, PREVIOUS_UNNAMED));

        }
        else if (label_name->variable() == Token::colon_plus.as_symbol()) {
            return Expression(std::make_shared<LabelExpression>(label_name->location, NEXT_UNNAMED));
        }
        else {
            throw ParseException(arguments[0].location(), "invalid arguments for .label_offset()");
        }
    }
    if (arguments.size() != 2) {
        throw ParseException(arguments.empty() ? Location() : arguments.front().location(), "invalid number of arguments for .label_offset()");
    }
    auto object_name = arguments[0].as_variable();
    auto label_name = arguments[1].as_variable();
    if (!object_name || !label_name) {
        throw ParseException(arguments[0].location(), "invalid arguments for .label_offset()");
    }
    return Expression(std::make_shared<LabelExpression>(object_name->location, object_name->variable(), label_name->variable()));
}

Expression LabelExpression::create(Location location, const Entity* object, Symbol label_name, SizeRange offset) {
    if (offset.size()) {
        return Expression(*offset.size());
    }
    else {
        return Expression(std::make_shared<LabelExpression>(location, object, label_name, offset));
    }
}


std::optional<Expression> LabelExpression::evaluated(const EvaluationContext& context) const {
    switch (type) {
        case NAMED: {
            const Entity* new_object = object;
            auto new_offset = offset;

            if (!object) {
                if (context.entity) {
                    if (object_name.empty() || object_name == context.entity->name.as_symbol()) {
                        new_object = context.entity;
                    }
                    else {
                        new_object = context.entity->owner->object(object_name); // TODO: search in included libraries too
                    }
                }
                else {
                    auto value = context.environment->get_variable(object_name);
                    if (value && value->is_object()) {
                        context.result.used_objects.insert(value->as_object()->object);
                        new_object = value->as_object()->object;
                    }
                    // TODO: handle unresolved label
                }
            }

            if (new_object) {
                // TODO: handle scope
                auto found_offset = new_object->environment->get_label(label_name);
                if (found_offset) {
                    new_offset = *found_offset;
                }
            }

            if (new_object != object || new_offset != offset) {
                    return Expression(location, new_object, label_name, new_offset);
            }
            break;
        }

        case NEXT_UNNAMED:
        case PREVIOUS_UNNAMED: {
            if (context.entity) {
                if (!added_to_environment()) {
                    auto new_unnamed_index = context.environment->unnamed_labels.add_user();
                    return Expression(location, type, new_unnamed_index);
                }
                else {
                    SizeRange new_offset;
                    if (type == NEXT_UNNAMED) {
                        new_offset = context.environment->unnamed_labels.get_next(unnamed_index);
                    }
                    else {
                        new_offset = context.environment->unnamed_labels.get_previous(unnamed_index);
                    }
                    if (new_offset != offset) {
                        return Expression(location, type, unnamed_index, new_offset);
                    }
                }
            }
            break;
        }
    }

    return {};
}

void LabelExpression::serialize_sub(std::ostream &stream) const {
    stream << ".label_offset(";
    switch (type) {
        case NAMED:
            if (object_name.empty()) {
                stream << ".current_object";
            }
            else {
                stream << object_name;
            }
            stream << ", " << label_name;
            break;

        case NEXT_UNNAMED:
            stream << ":+";
            break;

        case PREVIOUS_UNNAMED:
            stream << ":-";
    }
    stream << ")";
}
