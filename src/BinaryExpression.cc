/*
BinaryExpression.cc -- 

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

#include "BinaryExpression.h"

#include "VariableExpression.h"

std::optional<Expression> BinaryExpression::evaluated(const Environment &environment) const {
    auto new_left = left.evaluated(environment);
    auto new_right= right.evaluated(environment);

    if (!new_left && !new_right) {
        return {};
    }

    return Expression(new_left.value_or(left), operation, new_right.value_or(right));
}

void BinaryExpression::serialize_sub(std::ostream &stream) const {
    stream << '(' << left;

    switch (operation) {
        case Expression::BinaryOperation::ADD:
            stream << '+';
            break;

        case Expression::BinaryOperation::BITWISE_AND:
            stream << '&';
            break;

        case Expression::BinaryOperation::BITWISE_OR:
            stream << '|';
            break;

        case Expression::BinaryOperation::BITWISE_XOR:
            stream << '^';
            break;

        case Expression::BinaryOperation::DIVIDE:
            stream << '/';
            break;

        case Expression::BinaryOperation::LOGICAL_AND:
            stream << "&&";
            break;

        case Expression::BinaryOperation::LOGICAL_OR:
            stream << "||";
            break;

        case Expression::BinaryOperation::MODULO:
            stream << '%';
            break;

        case Expression::BinaryOperation::MULTIPLY:
            stream << '*';
            break;

        case Expression::BinaryOperation::SHIFT_LEFT:
            stream << "<<";
            break;

        case Expression::BinaryOperation::SHIFT_RIGHT:
            stream << ">>";
            break;

        case Expression::BinaryOperation::SUBTRACT:
            stream << '-';
            break;
    }

    stream << right << ')';
}


Expression BinaryExpression::create(const Expression& left, Expression::BinaryOperation operation, const Expression& right) {
    if (left.has_value() && right.has_value()) {
        auto left_value = *left.value();
        auto right_value = *right.value();
        Value value;

        switch (operation) {
            case Expression::BinaryOperation::ADD:
                value = left_value + right_value;
                break;

            case Expression::BinaryOperation::SUBTRACT:
                value = left_value - right_value;
                break;

            case Expression::BinaryOperation::SHIFT_RIGHT:
                value = left_value >> right_value;
                break;

            case Expression::BinaryOperation::SHIFT_LEFT:
                value = left_value << right_value;
                break;

            case Expression::BinaryOperation::BITWISE_XOR:
                value = left_value ^ right_value;
                break;

            case Expression::BinaryOperation::BITWISE_AND:
                value = left_value & right_value;
                break;

            case Expression::BinaryOperation::BITWISE_OR:
                value = left_value | right_value;
                break;

            case Expression::BinaryOperation::LOGICAL_AND:
                value = left_value && right_value;
                break;

            case Expression::BinaryOperation::LOGICAL_OR:
                value = left_value || right_value;
                break;

            case Expression::BinaryOperation::MULTIPLY:
                value = left_value * right_value;
                break;

            case Expression::BinaryOperation::DIVIDE:
                value = left_value / right_value;
                break;

            case Expression::BinaryOperation::MODULO:
                value = left_value % right_value;
                break;
        }

        return Expression(value);
    }
    else {
        switch (operation) {
            case Expression::BinaryOperation::ADD: {
                if (right.has_value() && right.value() == Value(uint64_t(0))) {
                    // N + 0 -> N
                    return left;
                }
                if (left.has_value() && left.value() == Value(uint64_t(0))) {
                    // 0 + N -> N
                    return right;
                }
                break;
            }

            case Expression::BinaryOperation::MULTIPLY: {
                if (left.has_value() && left.value() == Value(uint64_t(1))) {
                    // 0 * N -> 0
                    return left;
                }
                if (left.has_value() && left.value() == Value(uint64_t(1))) {
                    // 1 * N -> N
                    return right;
                }
                if (right.has_value() && right.value() == Value(uint64_t(1))) {
                    // N * 0 -> 0
                    return right;
                }
                if (right.has_value() && right.value() == Value(uint64_t(1))) {
                    // N * 1 -> N
                    return left;
                }
                break;
            }

            case Expression::BinaryOperation::DIVIDE: {
                if (right.has_value() && right.value() == Value(uint64_t(1))) {
                    // N / 1 -> N
                    return left;
                }
                break;
            }

            case Expression::BinaryOperation::SUBTRACT: {
                if (right.has_value() && right.value() == Value(uint64_t(0))) {
                    // N - 0 -> N
                    return left;
                }
                if (left.has_value() && left.value() == Value(uint64_t(0))) {
                    // 0 - N -> -N
                    return {Expression::UnaryOperation::MINUS, right};
                }
                // This special case is for resolving relative addressing within an object.
                if (left.is_binary() && right.is_binary()) {
                    // (object_name + N) - (object_name + M) -> N - M
                    auto left_binary = left.as_binary();
                    auto right_binary = right.as_binary();

                    if (left_binary->operation == Expression::BinaryOperation::ADD && right_binary->operation == Expression::BinaryOperation::ADD) {
                        auto left_variable = left_binary->left.variable_name();
                        auto right_variable = right_binary->left.variable_name();
                        if (!left_variable.empty() && left_variable == right_variable) {
                            return {left_binary->right, operation, right_binary->right};
                        }
                    }
                }
                else if (right.is_binary()) {
                    // object_name - (object_name + M) -> -M
                    auto left_variable = left.variable_name();
                    auto right_binary = right.as_binary();

                    if (!left_variable.empty() && right_binary->operation == Expression::BinaryOperation::ADD) {
                        auto right_variabel = right_binary->left.variable_name();
                        if (left_variable == right_variabel) {
                            return {Expression::UnaryOperation::MINUS, right_binary->right};
                        }
                    }
                }
                else if (left.is_binary()) {
                    // (object_name + N) - object_name -> N
                    auto left_binary = left.as_binary();
                    auto right_variable = right.variable_name();

                    if (!right_variable.empty() && left_binary->operation == Expression::BinaryOperation::ADD) {
                        auto left_variable = left_binary->left.variable_name();
                        if (left_variable == right_variable) {
                            return left_binary->right;
                        }
                    }
                }
                else {
                    // object_name - object_name -> 0
                    auto left_variable = left.variable_name();
                    auto right_variable = right.variable_name();
                    if (!left_variable.empty() && left_variable == right_variable) {
                        return Expression(uint64_t(0));
                    }
                }
                break;
            }

            case Expression::BinaryOperation::LOGICAL_AND:
                if (left.has_value() && *left.value()) {
                    return right;
                }
                if (right.has_value() && *right.value()) {
                    return left;
                }
                break;

            case Expression::BinaryOperation::LOGICAL_OR:
                if (left.has_value() && !*left.value()) {
                    return right;
                }
                if (right.has_value() && !*right.value()) {
                    return left;
                }
                break;

            default:
                break;
        }
    }
    return Expression(std::make_shared<BinaryExpression>(left, operation, right));
}

std::optional<Value> BinaryExpression::minimum_value() const {
    switch (operation) {
        case Expression::BinaryOperation::ADD:
            return left.minimum_value() + right.minimum_value();

        // TODO: calculate for more operations
        case Expression::BinaryOperation::BITWISE_AND:
        case Expression::BinaryOperation::BITWISE_OR:
        case Expression::BinaryOperation::BITWISE_XOR:
        case Expression::BinaryOperation::DIVIDE:
        case Expression::BinaryOperation::MODULO:
        case Expression::BinaryOperation::MULTIPLY:
        case Expression::BinaryOperation::SHIFT_LEFT:
        case Expression::BinaryOperation::SHIFT_RIGHT:
        case Expression::BinaryOperation::LOGICAL_AND:
        case Expression::BinaryOperation::LOGICAL_OR:
            return {};

        case Expression::BinaryOperation::SUBTRACT:
            return left.minimum_value() - right.maximum_value();
    }
}

std::optional<Value> BinaryExpression::maximum_value() const {
    switch (operation) {
        case Expression::BinaryOperation::ADD:
            return left.maximum_value() + right.maximum_value();

            // TODO: calculate for more operations
        case Expression::BinaryOperation::BITWISE_AND:
        case Expression::BinaryOperation::BITWISE_OR:
        case Expression::BinaryOperation::BITWISE_XOR:
        case Expression::BinaryOperation::DIVIDE:
        case Expression::BinaryOperation::MODULO:
        case Expression::BinaryOperation::MULTIPLY:
        case Expression::BinaryOperation::SHIFT_LEFT:
        case Expression::BinaryOperation::SHIFT_RIGHT:
        case Expression::BinaryOperation::LOGICAL_AND:
        case Expression::BinaryOperation::LOGICAL_OR:
            return {};

        case Expression::BinaryOperation::SUBTRACT:
            return left.maximum_value() - right.minimum_value();
    }
}
