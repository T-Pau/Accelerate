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

#include "ValueExpression.h"
#include "VariableExpression.h"

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left, Operation operation,
                                   std::shared_ptr<Expression> right): left(std::move(left)), operation(operation), right(std::move(right)) {}


std::shared_ptr<Expression> BinaryExpression::evaluate(const Environment &environment) const {
    auto new_left = left->evaluate(environment);
    auto new_right= right->evaluate(environment);

    if (!new_left && !new_right) {
        return {};
    }

    return create(new_left ? new_left : left, operation, new_right ? new_right : right, byte_size());
}

void BinaryExpression::serialize_sub(std::ostream &stream) const {
    stream << '(' << left;

    switch (operation) {
        case ADD:
            stream << '+';
            break;

        case BITWISE_AND:
            stream << '&';
            break;

        case BITWISE_OR:
            stream << '|';
            break;

        case BITWISE_XOR:
            stream << '^';
            break;

        case DIVIDE:
            stream << '/';
            break;

        case MODULO:
            stream << '%';
            break;

        case MULTIPLY:
            stream << '*';
            break;

        case SHIFT_LEFT:
            stream << "<<";
            break;

        case SHIFT_RIGHT:
            stream << ">>";
            break;

        case SUBTRACT:
            stream << '-';
            break;
    }

    stream << right << ')';
}

size_t BinaryExpression::minimum_byte_size() const {
    auto left_size = left->minimum_byte_size();
    auto right_size = right->minimum_byte_size();

    if (left_size == 0 || right_size == 0) {
        return 0;
    }

    switch (operation) {
        case ADD:
        case SUBTRACT:
        case DIVIDE:
        case MULTIPLY:
        case SHIFT_RIGHT:
            return 0;

        case BITWISE_AND:
        case BITWISE_OR:
        case BITWISE_XOR:
            return std::max(left_size, right_size);

        case MODULO:
            return right_size;

        case SHIFT_LEFT:
            return left_size;
    }
}


std::shared_ptr<Expression> BinaryExpression::create(const std::shared_ptr<Expression>& left, Operation operation, const std::shared_ptr<Expression>& right, size_t byte_size) {
    std::shared_ptr<Expression> node;

    if (left->has_value() && right->has_value()) {
        auto left_value = left->value();
        auto right_value = right->value();
        Value value;

        switch (operation) {
            case ADD:
                value = left_value + right_value;
                break;

            case SUBTRACT:
                value = left_value - right_value;
                break;

            case SHIFT_RIGHT:
                value = left_value >> right_value;
                break;

            case SHIFT_LEFT:
                value = left_value << right_value;
                break;

            case BITWISE_XOR:
                value = left_value ^ right_value; // TODO: mask with size
                break;

            case BITWISE_AND:
                value = left_value & right_value;
                break;

            case BITWISE_OR:
                value = left_value | right_value;
                break;

            case MULTIPLY:
                value = left_value * right_value;
                break;

            case DIVIDE:
                value = left_value / right_value;
                break;

            case MODULO:
                value = left_value % right_value;
                break;
        }

        // TODO: check byte_size

        node = std::make_shared<ValueExpression>(value);
    }
    else if (operation == SUBTRACT) {
        // This special case is for resolving relative addressing within an object.
        if (left->type() == BINARY && right->type() == BINARY) {
            // (object_name + N) - (object_name + M) -> N - M
            auto left_binary = std::dynamic_pointer_cast<BinaryExpression>(left);
            auto right_binary = std::dynamic_pointer_cast<BinaryExpression>(right);

            if (left_binary->operation == ADD && right_binary->operation == ADD &&
                left_binary->left->type() == VARIABLE && right_binary->left->type() == VARIABLE) {
                auto left_left = std::dynamic_pointer_cast<VariableExpression>(left_binary->left);
                auto right_left = std::dynamic_pointer_cast<VariableExpression>(right_binary->left);
                if (left_left->variable() == right_left->variable()) {
                    node = create(left_binary->right, operation, right_binary->right, byte_size);
                }
            }
        }
        else if (left->type() == VARIABLE && right->type() == BINARY) {
            // object_name - (object_name + M) -> M
            auto left_variable = std::dynamic_pointer_cast<VariableExpression>(left);
            auto right_binary = std::dynamic_pointer_cast<BinaryExpression>(right);

            if (right_binary->operation == ADD && right_binary->left->type() == VARIABLE) {
                auto right_left = std::dynamic_pointer_cast<VariableExpression>(right_binary->left);
                if (left_variable->variable() == right_left->variable()) {
                    node = right_binary->right->clone();
                }
            }
        }
        else if (left->type() == BINARY && right->type() == VARIABLE) {
            // (object_name + N) - object_name -> N
            auto left_binary = std::dynamic_pointer_cast<BinaryExpression>(left);
            auto right_variable = std::dynamic_pointer_cast<VariableExpression>(right);

            if (left_binary->operation == ADD && left_binary->left->type() == VARIABLE) {
                auto left_left = std::dynamic_pointer_cast<VariableExpression>(left_binary->left);
                if (left_left->variable() == right_variable->variable()) {
                    node = left_binary->right->clone();
                }
            }
        }
        else if (left->type() == VARIABLE && right->type() == VARIABLE) {
            // object_name - object_name -> 0
            auto left_variable = std::dynamic_pointer_cast<VariableExpression>(left);
            auto right_variable = std::dynamic_pointer_cast<VariableExpression>(right);
            if (left_variable->variable() == right_variable->variable()) {
                node = std::make_shared<ValueExpression>(0);
            }
        }
    }

    // TODO: optimization for + or - 0, * or / 1

    if (!node) {
        node = std::make_shared<BinaryExpression>(left, operation, right);
    }

    if (byte_size) {
        node->set_byte_size(byte_size);
    }
    return node;
}
