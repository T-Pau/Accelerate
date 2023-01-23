/*
ExpressionNode.cc -- Abstract Base Class of Expression Nodes

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

#include "ExpressionNode.h"

#include <utility>
#include "ParseException.h"

std::shared_ptr<ExpressionNode> ExpressionNode::evaluate(std::shared_ptr<ExpressionNode> node, const Environment &environment) {
    auto new_node = node->evaluate(environment);

    if (!new_node) {
        return node;
    }
    else {
        return new_node;
    }
}

std::shared_ptr<ExpressionNode> ExpressionNode::create_binary(const std::shared_ptr<ExpressionNode>& left, ExpressionNode::SubType operation, const std::shared_ptr<ExpressionNode>& right) {
    if (left->subtype() == INTEGER && right->subtype() == INTEGER) {
        auto left_value = std::dynamic_pointer_cast<ExpressionNodeInteger>(left)->as_int();
        auto right_value = std::dynamic_pointer_cast<ExpressionNodeInteger>(right)->as_int();
        int64_t value;

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

            default:
                throw Exception("invalid operand for binary expression");
        }

        return std::make_shared<ExpressionNodeInteger>(value);
    }
    else {
        return std::make_shared<ExpressionNodeBinary>(left, operation, right);
    }
}

std::shared_ptr<ExpressionNode> ExpressionNode::create_unary(ExpressionNode::SubType operation, std::shared_ptr<ExpressionNode> operand) {
    if (operation == PLUS) {
        return operand;
    }
    else if (operand->subtype() == INTEGER) {
        auto value = std::dynamic_pointer_cast<ExpressionNodeInteger>(operand)->as_int();
        switch (operation) {
            case MINUS:
                value = -value;
                break;

            case BITWISE_NOT:
                value = ~value; // TODO: mask to size
                break;

            case LOW_BYTE:
                value =value & 0xff;
                break;

            case HIGH_BYTE:
                value = (value >> 8) & 0xff;
                break;

            case BANK_BYTE:
                value = (value >> 16) & 0xff;
                break;

            default:
                throw Exception("invalid operand for unary expression");
        }

        return std::make_shared<ExpressionNodeInteger>(value);
    }
    else {
        return std::make_shared<ExpressionNodeUnary>(operation, operand);
    }
}


ExpressionNodeInteger::ExpressionNodeInteger(const Token &token) {
    if (!token.is_integer()) {
        throw ParseException(token, "internal error: can't create integer node from %s", token.type_name());
    }
    value = static_cast<int64_t>(token.as_integer()); // TODO: handle overflow
}

size_t ExpressionNodeInteger::byte_size() const {
    return 0; // TODO
}

size_t ExpressionNodeInteger::minimum_size() const {
    if (value > std::numeric_limits<uint32_t>::max()) {
        return 8;
    }
    else if (value > std::numeric_limits<uint16_t>::max()) {
        return 4;
    }
    else if (value > std::numeric_limits<uint8_t>::max()) {
        return 2;
    }
    else {
        return 1;
    }
}

ExpressionNodeVariable::ExpressionNodeVariable(const Token &token) {
    if (!token.is_name()) {
        throw Exception("internal error: creating ExpressionNodeVariable from non-name token");
    }
    location = token.location;
    symbol = token.as_symbol();
}

std::shared_ptr<ExpressionNode> ExpressionNodeVariable::evaluate(const Environment &environment) const {
    auto value = environment[symbol];

    if (value) {
        // TODO: Don't evaluate further if evaluating encoding.
        // TODO: Detect loops
        return ExpressionNode::evaluate(value, environment);
    }
    else {
        return {};
    }
}


ExpressionNodeUnary::ExpressionNodeUnary(SubType operation, std::shared_ptr<ExpressionNode>operand) : operation(operation), operand(std::move(operand)) {
    switch (operation) {
        case MINUS:
        case BITWISE_NOT:
        case LOW_BYTE:
        case HIGH_BYTE:
        case BANK_BYTE:
            return;

        default:
            throw Exception("invalid operand for unary expression");
    }
}

std::shared_ptr<ExpressionNode> ExpressionNodeUnary::evaluate(const Environment &environment) const {
    auto new_operand = operand->evaluate(environment);

    if (!new_operand) {
        return {};
    }

    return create_unary(operation, new_operand ? new_operand : operand);
}


ExpressionNodeBinary::ExpressionNodeBinary(std::shared_ptr<ExpressionNode> left, ExpressionNode::SubType operation,
                                           std::shared_ptr<ExpressionNode> right): left(std::move(left)), operation(operation), right(std::move(right)) {
    switch (operation) {
        case ADD:
        case SUBTRACT:
        case SHIFT_RIGHT:
        case SHIFT_LEFT:
        case BITWISE_XOR:
        case BITWISE_AND:
        case BITWISE_OR:
        case MULTIPLY:
        case DIVIDE:
        case MODULO:
            return;

        default:
            throw Exception("invalid operand for binary expression");
    }
}


std::shared_ptr<ExpressionNode> ExpressionNodeBinary::evaluate(const Environment &environment) const {
    auto new_left = left->evaluate(environment);
    auto new_right= right->evaluate(environment);

    if (!new_left && !new_right) {
        return {};
    }

    return create_binary(new_left ? new_left : left, operation, new_right ? new_right : right);
}
