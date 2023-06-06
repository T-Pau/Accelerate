//
// Created by Dieter Baron on 03.05.23.
//

#include "LabelExpression.h"

#include "Expression.h"
#include "Object.h"
#include "ObjectFile.h"
#include "ParseException.h"
#include "VariableExpression.h"

Expression LabelExpression::create(const std::vector<Expression>& arguments) {
    if (arguments.size() != 2) {
        throw ParseException(Location(), "invalid number of arguments for .label_offset()"); // TODO: location
    }
    auto object_name = arguments[0].as_variable();
    auto label_name = arguments[1].as_variable();
    if (!object_name || !label_name) {
        throw ParseException(Location(), "invalid arguments for .label_offset()");
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

            if (!object && context.object) {
                if (unresolved_object_name.empty() || unresolved_object_name == context.object->name.as_symbol()) {
                    new_object = context.object;
                }
                else {
                    new_object = context.object->owner->object(unresolved_object_name); // TODO: search in included libraries too
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
            context.scope->add_forward_unnamed_label_use(this);
            break;

        case PREVIOUS_UNNAMED:
            try {
                label = context.scope->get_previous_unnamed_label();
                type = NAMED;
            }
            catch (Exception &ex) {
                if (!context.conditional_in_scope) {
                    throw ex;
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
    return object ? object->name.as_symbol() : unresolved_label_name;
}