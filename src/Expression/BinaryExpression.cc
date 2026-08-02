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

#include "Expression/BinaryExpression.h"

#include <tpau-cpp-kernal/Exception.h>

#include "Expression/ConstantExpression.h"
#include "Expression/LabelOffsetExpression.h"
#include "Expression/ObjectExpression.h"
#include "Expression/UnaryExpression.h"
#include "Expression/ValueExpression.h"

using namespace tpau::cpp_kernal;

// Keep in sync with Expression::BinaryOperation
// clang-format off
std::vector<std::string> BinaryExpression::operation_names = {
    "+",
    "&",
    "|", 
    "^", 
    "/",
    "=",
    ">",
    ">=",
    "<",
    "<=",
    "&&",
    "||", 
    "%",
    "*",
    "!=",
    "<<",
    ">>",
    "-"
};
// clang-format on

std::optional<Expression> BinaryExpression::evaluate_process(const EvaluationContext& context) { return simplify(location, left, operation, right, false); }

void BinaryExpression::serialize_sub(std::ostream& stream) const { stream << '(' << left << operation_name() << right << ')'; }

std::optional<Expression> BinaryExpression::simplify(const Location& location, const Expression& left, Operation operation, const Expression& right, bool always_create) {
    // TODO: check that types are compatible

    if (left.has_value() && right.has_value()) {
        auto left_value = *left.value();
        auto right_value = *right.value();
        Value value;

        switch (operation) {
            case Operation::ADD:
                value = left_value + right_value;
                break;

            case Operation::SUBTRACT:
                value = left_value - right_value;
                break;

            case Operation::SHIFT_RIGHT:
                value = left_value >> right_value;
                break;

            case Operation::SHIFT_LEFT:
                value = left_value << right_value;
                break;

            case Operation::BITWISE_XOR:
                value = left_value ^ right_value;
                break;

            case Operation::BITWISE_AND:
                value = left_value & right_value;
                break;

            case Operation::BITWISE_OR:
                value = left_value | right_value;
                break;

            case Operation::LOGICAL_AND:
                value = left_value && right_value;
                break;

            case Operation::LOGICAL_OR:
                value = left_value || right_value;
                break;

            case Operation::MULTIPLY:
                value = left_value * right_value;
                break;

            case Operation::DIVIDE:
                value = left_value / right_value;
                break;

            case Operation::MODULO:
                value = left_value % right_value;
                break;
            case Operation::EQUAL:
                value = Value(left_value == right_value);
                break;

            case Operation::GREATER:
                value = Value(left_value > right_value);
                break;

            case Operation::GREATER_EQUAL:
                value = Value(left_value >= right_value);
                break;

            case Operation::LESS:
                value = Value(left_value < right_value);
                break;

            case Operation::LESS_EQUAL:
                value = Value(left_value <= right_value);
                break;

            case Operation::NOT_EQUAL:
                value = Value(left_value != right_value);
                break;
        }

        return ValueExpression::create(location, value);
    }
    else {
        switch (operation) {
            case Operation::ADD: {
                if (right.has_value()) {
                    auto right_value = *right.value();
                    if (right_value == Value(uint64_t{0})) {
                        // N + 0 -> N
                        return left;
                    }
                    else if (left.is<BinaryExpression>()) {
                        auto left_binary = left.as<BinaryExpression>();
                        if (left_binary->right.has_value()) {
                            auto left_right_value = *left_binary->right.value();
                            if (left_binary->operation == Operation::ADD) {
                                // (N + A) + B -> N + (A+B)
                                auto new_right = ValueExpression::create({left_binary->right.location(), right.location()}, left_right_value + right_value);
                                return BinaryExpression::create(location, left_binary->left, Operation::ADD, new_right);
                            }
                            else if (left_binary->operation == Operation::SUBTRACT) {
                                // (N - A) + B -> N + (B-A)
                                auto new_right = ValueExpression::create({left_binary->right.location(), right.location()}, right_value - left_right_value);
                                return BinaryExpression::create(location, left_binary->left, Operation::ADD, new_right);
                            }
                        }
                    }
                }
                if (left.has_value()) {
                    auto left_value = *left.value();
                    if (left_value == Value(uint64_t{0})) {
                        // 0 + N -> N
                        return right;
                    }
                    else if (right.is<BinaryExpression>()) {
                        auto right_binary = right.as<BinaryExpression>();
                        if (right_binary->left.has_value()) {
                            auto right_left_value = *right_binary->left.value();
                            if (right_binary->operation == Operation::ADD) {
                                // A + (B + N) -> N + (A+B)
                                auto new_right = ValueExpression::create({left.location(), right_binary->left.location()}, left_value + right_left_value);
                                return BinaryExpression::create(location, right_binary->right, Operation::ADD, new_right);
                            }
                            else if (right_binary->operation == Operation::SUBTRACT) {
                                // A + (B - N) -> (A+B) - N
                                auto new_left = ValueExpression::create({left.location(), right_binary->left.location()}, left_value + right_left_value);
                                return BinaryExpression::create(location, new_left, Operation::SUBTRACT, right_binary->right);
                            }
                        }
                    }
                }
                break;
            }

            case Operation::EQUAL:
                if (left.minimum_value() > right.maximum_value() || left.maximum_value() < right.minimum_value()) {
                    return ValueExpression::create(location, Value(false));
                }
                break;

            case Operation::GREATER:
                if (left.minimum_value() > right.maximum_value()) {
                    return ValueExpression::create(location, Value(true));
                }
                else if (left.maximum_value() <= right.minimum_value()) {
                    return ValueExpression::create(location, Value(false));
                }
                break;

            case Operation::GREATER_EQUAL:
                if (left.minimum_value() >= right.maximum_value()) {
                    return ValueExpression::create(location, Value(true));
                }
                else if (left.maximum_value() < right.minimum_value()) {
                    return ValueExpression::create(location, Value(false));
                }
                break;

            case Operation::LESS:
                if (left.minimum_value() >= right.maximum_value()) {
                    return ValueExpression::create(location, Value(false));
                }
                else if (left.maximum_value() < right.minimum_value()) {
                    return ValueExpression::create(location, Value(true));
                }
                break;

            case Operation::LESS_EQUAL:
                if (left.minimum_value() > right.maximum_value()) {
                    return ValueExpression::create(location, Value(false));
                }
                else if (left.maximum_value() <= right.minimum_value()) {
                    return ValueExpression::create(location, Value(true));
                }
                break;

            case Operation::MULTIPLY: {
                if (left.has_value() && left.value() == Value(uint64_t{1})) {
                    // 0 * N -> 0
                    return left;
                }
                if (left.has_value() && left.value() == Value(uint64_t{1})) {
                    // 1 * N -> N
                    return right;
                }
                if (right.has_value() && right.value() == Value(uint64_t{1})) {
                    // N * 0 -> 0
                    return right;
                }
                if (right.has_value() && right.value() == Value(uint64_t{1})) {
                    // N * 1 -> N
                    return left;
                }
                break;
            }

            case Operation::NOT_EQUAL:
                if (left.minimum_value() > right.maximum_value() || left.maximum_value() < right.minimum_value()) {
                    return ValueExpression::create(location, Value(true));
                }
                break;

            case Operation::DIVIDE: {
                if (right.has_value() && right.value() == Value(uint64_t{1})) {
                    // N / 1 -> N
                    return left;
                }
                break;
            }

            case Operation::SUBTRACT: {
                if (right.has_value() && right.value() == Value(uint64_t{0})) {
                    // N - 0 -> N
                    return left;
                }
                if (left.has_value() && left.value() == Value(uint64_t{0})) {
                    // 0 - N -> -N
                    return UnaryExpression::create(location, UnaryExpression::Operation::MINUS, right);
                }

                // For the difference of two label expressions within the same object, return the difference of their offsets.
                // This simplifies relative jumps within an object.
                auto [left_entity, left_offset] = deconstruct_label(left);
                auto [right_entity, right_offset] = deconstruct_label(right);
                if (left_entity && right_entity) {
                    if (left_entity == right_entity) {
                        if (left_offset && right_offset) {
                            auto left_label_offset = left_offset->as<LabelOffsetExpression>();
                            auto right_label_offset = right_offset->as<LabelOffsetExpression>();
                            if (left_label_offset && right_label_offset && *left_label_offset == *right_label_offset) {
                                // The same label offset, so the difference is 0.
                                return ValueExpression::create(location, Value(uint64_t{0}));
                            }
                            // left_offset - right_offset
                            return BinaryExpression::create(location, *left_offset, Operation::SUBTRACT, *right_offset);
                        }
                        else if (left_offset) {
                            // left_offset - 0
                            return *left_offset;
                        }
                        else if (right_offset) {
                            // 0 - right_offset
                            return UnaryExpression::create(location, UnaryExpression::Operation::MINUS, *right_offset);
                        }
                        else {
                            // 0 - 0
                            return ValueExpression::create(location, Value(uint64_t{0}));
                        }
                    }
                }
                break;
            }

            case Operation::LOGICAL_AND:
                // true && X -> X
                if (left.has_value() && *left.value()) {
                    return right;
                }
                // X && true -> X
                if (right.has_value() && *right.value()) {
                    return left;
                }
                break;

            case Operation::LOGICAL_OR:
                // false || X -> X
                if (left.has_value() && !*left.value()) {
                    return right;
                }
                // X || false -> X
                if (right.has_value() && !*right.value()) {
                    return left;
                }
                break;

            case Operation::BITWISE_AND:
                // 0 & X -> 0
                if (left.has_value() && left.value()->unsigned_value() == 0) {
                    return left;
                }
                // X & 0 -> 0
                if (right.has_value() && right.value()->unsigned_value() == 0) {
                    return right;
                }
                break;

            case Operation::BITWISE_OR:
            case Operation::BITWISE_XOR:
                // 0 | X -> X
                if (left.has_value() && left.value()->unsigned_value() == 0) {
                    return right;
                }
                // X | 0 -> X
                if (right.has_value() && right.value()->unsigned_value() == 0) {
                    return left;
                }
                break;

            default:
                break;
        }
    }
    if (always_create) {
        return Expression(std::make_shared<BinaryExpression>(location, left, operation, right));
    }
    else {
        return {};
    }
}

std::optional<Value> BinaryExpression::minimum_value() const {
    switch (operation) {
        case Operation::ADD:
            return left.minimum_value() + right.minimum_value();

        // TODO: calculate for more operations
        case Operation::BITWISE_AND:
        case Operation::BITWISE_OR:
        case Operation::BITWISE_XOR:
        case Operation::DIVIDE:
        case Operation::LOGICAL_AND:
        case Operation::LOGICAL_OR:
        case Operation::MODULO:
        case Operation::MULTIPLY:
        case Operation::SHIFT_LEFT:
        case Operation::SHIFT_RIGHT:
        case Operation::EQUAL:
        case Operation::GREATER:
        case Operation::GREATER_EQUAL:
        case Operation::LESS:
        case Operation::LESS_EQUAL:
        case Operation::NOT_EQUAL:
            return {};

        case Operation::SUBTRACT:
            return left.minimum_value() - right.maximum_value();
    }

    throw Exception("internal error: invalid binary operation {}", static_cast<int>(operation));
}

std::optional<Value> BinaryExpression::maximum_value() const {
    switch (operation) {
        case Operation::ADD:
            return left.maximum_value() + right.maximum_value();

            // TODO: calculate for more operations
        case Operation::BITWISE_AND:
        case Operation::BITWISE_OR:
        case Operation::BITWISE_XOR:
        case Operation::DIVIDE:
        case Operation::LOGICAL_AND:
        case Operation::LOGICAL_OR:
        case Operation::MODULO:
        case Operation::MULTIPLY:
        case Operation::SHIFT_LEFT:
        case Operation::SHIFT_RIGHT:
        case Operation::EQUAL:
        case Operation::GREATER:
        case Operation::GREATER_EQUAL:
        case Operation::LESS:
        case Operation::LESS_EQUAL:
        case Operation::NOT_EQUAL:
            return {};

        case Operation::SUBTRACT:
            return left.maximum_value() - right.minimum_value();
    }

    throw Exception("internal error: invalid binary operation {}", static_cast<int>(operation));
}

std::optional<Value::Type> BinaryExpression::type() const {
    const auto left_type = left.type();
    const auto right_type = right.type();

    switch (operation) {
        case Operation::ADD:
        case Operation::SUBTRACT:
        case Operation::MULTIPLY:
        case Operation::DIVIDE:
            if (left_type && right_type) {
                if (left_type == Value::FLOAT || right_type == Value::FLOAT) {
                    return Value::FLOAT;
                }
                else if (left_type == Value::UNSIGNED && right_type == Value::UNSIGNED) {
                    return Value::UNSIGNED;
                }
                else {
                    return Value::INTEGER;
                }
            }
            else {
                return Value::NUMBER;
            }

        case Operation::MODULO:
            // TODO
            return {};

        case Operation::SHIFT_LEFT:
        case Operation::SHIFT_RIGHT:
            if (left_type) {
                return left_type;
            }
            else {
                return Value::INTEGER;
            }

        case Operation::BITWISE_AND:
        case Operation::BITWISE_OR:
        case Operation::BITWISE_XOR:
            return Value::UNSIGNED;

        case Operation::EQUAL:
        case Operation::GREATER:
        case Operation::GREATER_EQUAL:
        case Operation::LESS:
        case Operation::LESS_EQUAL:
        case Operation::NOT_EQUAL:
        case Operation::LOGICAL_AND:
        case Operation::LOGICAL_OR:
            return Value::BOOLEAN;
    }

    throw Exception("internal error: invalid binary operation {}", static_cast<int>(operation));
}

const std::string& BinaryExpression::operation_name(Operation operation) {
    if (static_cast<size_t>(operation) >= operation_names.size()) {
        throw Exception("internal error: invalid binary operation {}", static_cast<int>(operation));
    }
    return operation_names[static_cast<size_t>(operation)];
}

std::optional<BinaryExpression::LabelEntity> BinaryExpression::get_label_entity(const Expression& expression) {
    // TODO: handle macros
    if (auto object_expression = expression.as<ObjectExpression>()) {
        return object_expression->object();
    }
    else if (auto value_expression = expression.as<ValueExpression>()) {
        auto value = *value_expression->value();
        if (value.is_unsigned()) {
            return value.unsigned_value();
        }
    }
    else if (auto constant_expression = expression.as<ConstantExpression>()) {
        if (auto constant = constant_expression->constant()) {
            if (auto object_expression = constant->value.as<ObjectExpression>()) {
                return object_expression->object();
            }
            else if (auto value_expression = constant->value.as<ValueExpression>()) {
                auto value = *value_expression->value();
                if (value.is_unsigned()) {
                    return value.unsigned_value();
                }
            }
        }
    }
    return {};
}

std::pair<std::optional<BinaryExpression::LabelEntity>, std::optional<Expression>> BinaryExpression::deconstruct_label(const Expression& expression) {
    if (auto entity = get_label_entity(expression)) {
        TRACE_INSTANCE(expression, "deconstructing label", "{} {} {}", entity->type_name(), static_cast<void*>(entity), entity->name);
        return {entity, {}};
    }
    else if (auto constant_expression = expression.as<ConstantExpression>()) {
        if (auto constant = constant_expression->constant()) {
            if (auto binary_expression = constant->value.as<BinaryExpression>()) {
                if (binary_expression->operation != Operation::ADD) {
                    return {{}, {}};
                }
                else if (auto entity = get_label_entity(binary_expression->left)) {
                    TRACE_INSTANCE(expression, "deconstructing label", "{} {} {}, offset {}", entity->type_name(), static_cast<void*>(entity), entity->name, binary_expression->right);
                    return {entity, binary_expression->right};
                }
                else {
                    return {{}, {}};
                }
            }
        }
    }
    return {{}, {}};
}

bool BinaryExpression::LabelEntity::operator==(const LabelEntity& other) const {
    if (entity_or_address.index() != other.entity_or_address.index()) {
        return false;
    }
    if (std::holds_alternative<Entity*>(entity_or_address)) {
        return std::get<Entity*>(entity_or_address) == std::get<Entity*>(other.entity_or_address);
    }
    else {
        return std::get<uint64_t>(entity_or_address) == std::get<uint64_t>(other.entity_or_address);
    }
}
