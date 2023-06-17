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

#include "Expression.h"
#include "Object.h"
#include "ObjectExpression.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "VariableExpression.h"

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

Expression LabelExpression::create(Location location, const Object* object, std::shared_ptr<Label> label) {
    auto offset = label->offset.size();
    if (offset.has_value()) {
        return Expression(*offset);
    }
    else {
        return Expression(std::make_shared<LabelExpression>(location, object, std::move(label)));
    }
}

Expression LabelExpression::create(Location location, const Object* object, Symbol label_name) {
    auto label = object->environment->get_label(label_name);
    if (label) {
        return create(location, object, label);
    }
    else {
        return Expression(std::make_shared<LabelExpression>(location, object, label_name));
    }
}

std::optional<Expression> LabelExpression::evaluated(const EvaluationContext& context) const {
    switch (type) {
        case NAMED: {
            auto changed = false;
            const Object* new_object = object;
            auto new_label = label;

            if (!object) {
                if (context.object) {
                    if (unresolved_object_name.empty() || unresolved_object_name == context.object->name.as_symbol()) {
                        new_object = context.object;
                    }
                    else {
                        new_object = context.object->owner->object(unresolved_object_name); // TODO: search in included libraries too
                    }
                }
                else {
                    auto value = context.environment->get_variable(unresolved_object_name);
                    if (value && value->is_object()) {
                        new_object = value->as_object()->object;
                    }
                }

                if (new_object) {
                    changed = true;
                }
            }

            if (new_object && !label) {
                // TODO: handle scope
                new_label = new_object->environment->get_label(unresolved_label_name);
                if (new_label) {
                    changed = true;
                }
            }
            if (changed) {
                if (new_object) {
                    if (new_label) {
                        return Expression(location, new_object, new_label);
                    }
                    else {
                        return Expression(location, new_object, label_name());
                    }
                }
                else {
                    if (new_label) {
                        return Expression(location, object_name(), new_label);
                    }
                    else {
                        return Expression(location, object_name(), label_name());
                    }
                }
            }
            break;
        }

        case NEXT_UNNAMED:
            if (!label && context.object) {
                context.scope->add_forward_unnamed_label_use(this);
            }
            break;

        case PREVIOUS_UNNAMED:
            if (!label && context.object) {
                try {
                    label = context.scope->get_previous_unnamed_label();
                }
                catch (Exception& ex) {
                    if (!context.conditional_in_scope) {
                        throw ex;
                    }
                }
            }
            break;
    }

    if (label) {
        auto offset = label->offset.size();
        if (offset.has_value()) {
            return Expression(*offset);
        }
    }
    return {};
}

void LabelExpression::serialize_sub(std::ostream &stream) const {
    if (label && label->offset.size()) {
        stream << *label->offset.size();
    }
    else {
        stream << ".label_offset(";
        switch (type) {
            case NAMED:
                if (object_name().empty()) {
                    stream << ".current_object";
                }
                else {
                    stream << object_name();
                }
                stream << ", " << label_name();
                break;

            case NEXT_UNNAMED:
                stream << ":+";
                break;

            case PREVIOUS_UNNAMED:
                stream << ":-";
        }
        stream << ")";
    }
}


std::optional<Value> LabelExpression::maximum_value() const {
    if (!label) {
        return {};
    }
    auto v = label->offset.maximum;

    if (v) {
        return Value(*v);
    }
    else {
        return {};
    }
}

std::optional<Value> LabelExpression::value() const {
    if (!label) {
        return {};
    }
    auto offset = label->offset.size();

    if (offset) {
        return Value(*offset);
    }
    else {
        return {};
    }
}

std::optional<Value> LabelExpression::minimum_value() const {
    if (!label) {
        return Value(uint64_t(0));
    }
    return Value(label->offset.minimum);
}

Symbol LabelExpression::object_name() const {
    return object ? object->name.as_symbol() : unresolved_object_name;
}