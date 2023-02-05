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


std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<ExpressionNode>& node) {
    node->serialize(stream);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const ExpressionNode& node) {
    node.serialize(stream);
    return stream;
}


std::shared_ptr<ExpressionNode> ExpressionNode::evaluate(std::shared_ptr<ExpressionNode> node, const Environment &environment) {
    auto new_node = node->evaluate(environment);

    if (!new_node) {
        return node;
    }
    else {
        return new_node;
    }
}

std::shared_ptr<ExpressionNode> ExpressionNode::create_binary(const std::shared_ptr<ExpressionNode>& left, ExpressionNode::SubType operation, const std::shared_ptr<ExpressionNode>& right, size_t byte_size) {
    std::shared_ptr<ExpressionNode> node;

    if (left->has_value() && right->has_value()) {
        auto left_value = std::dynamic_pointer_cast<ExpressionNodeInteger>(left)->value();
        auto right_value = std::dynamic_pointer_cast<ExpressionNodeInteger>(right)->value();
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

        // TODO: check byte_size

        node = std::make_shared<ExpressionNodeInteger>(value);
    }
    else {
        node = std::make_shared<ExpressionNodeBinary>(left, operation, right);
    }

    node->set_byte_size(byte_size);
    return node;
}

std::shared_ptr<ExpressionNode> ExpressionNode::create_unary(ExpressionNode::SubType operation, std::shared_ptr<ExpressionNode> operand, size_t byte_size) {
    std::shared_ptr<ExpressionNode> node;

    if (operation == PLUS) {
        return operand;
    }
    else if (operand->has_value()) {
        auto value = std::dynamic_pointer_cast<ExpressionNodeInteger>(operand)->value();
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

        node = std::make_shared<ExpressionNodeInteger>(value);
    }
    else {
        node = std::make_shared<ExpressionNodeUnary>(operation, operand);
    }

    node->set_byte_size(byte_size);
    return node;
}


void ExpressionNode::set_byte_size(size_t size) {
    if (size != 0 && minimum_byte_size() != 0 && size < minimum_byte_size()) {
        throw ParseException(location, "value overflow");
    }
    byte_size_ = size;
}

void ExpressionNode::serialize(std::ostream &stream) const {
    serialize_sub(stream);
    if (byte_size() != 0 && (minimum_byte_size() == 0 || minimum_byte_size() != byte_size())) {
        stream << ":" << byte_size();
    }
}

std::shared_ptr<ExpressionNode> ExpressionNodeUnary::clone() const {
    switch (subtype()) {
        case BANK_BYTE:
        case BITWISE_NOT:
        case HIGH_BYTE:
        case LOW_BYTE:
        case MINUS:
            return create_unary(operation, operand, byte_size());
            break;

        case ADD:
        case BITWISE_AND:
        case BITWISE_OR:
        case BITWISE_XOR:
        case DIVIDE:
        case MODULO:
        case MULTIPLY:
        case PLUS:
        case SHIFT_LEFT:
        case SHIFT_RIGHT:
        case SUBTRACT:
        case INTEGER:
        case SIZE:
        case VARIABLE:
            throw ParseException(location, "internal error: invalid binary operation");
    }
}

std::shared_ptr<ExpressionNode> ExpressionNodeBinary::clone() const {
    switch (subtype()) {
        case ADD:
        case BITWISE_AND:
        case BITWISE_OR:
        case BITWISE_XOR:
        case DIVIDE:
        case MODULO:
        case MULTIPLY:
        case PLUS:
        case SHIFT_LEFT:
        case SHIFT_RIGHT:
        case SUBTRACT:
            return create_binary(left, operation, right, byte_size());


        case BANK_BYTE:
        case BITWISE_NOT:
        case HIGH_BYTE:
        case LOW_BYTE:
        case MINUS:
        case INTEGER:
        case SIZE:
        case VARIABLE:
            throw ParseException(location, "internal error: invalid binary operation");
    }
}


ExpressionNodeInteger::ExpressionNodeInteger(const Token &token) {
    if (!token.is_integer()) {
        throw ParseException(token, "internal error: can't create integer node from %s", token.type_name());
    }
    value_ = static_cast<int64_t>(token.as_integer()); // TODO: handle overflow
}

std::shared_ptr<ExpressionNode> ExpressionNodeInteger::clone() const {
    auto node = std::make_shared<ExpressionNodeInteger>(value());
    node->set_byte_size(byte_size());
    return node;
}


std::shared_ptr<ExpressionNode> ExpressionNodeVariable::clone() const {
    auto node = std::make_shared<ExpressionNodeInteger>(symbol);
    node->set_byte_size(byte_size());
    return node;
}


void ExpressionNodeInteger::serialize_sub(std::ostream &stream) const {
    auto width = static_cast<int>(byte_size() > 0 ? byte_size() : minimum_byte_size()) * 2;
    stream << "$" << std::setfill('0') << std::setw(width) << std::hex << value();
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
        auto node = ExpressionNode::evaluate(value, environment);

        if (byte_size() != 0 && node->byte_size() != byte_size()) {
            node = node->clone();
            node->set_byte_size(byte_size());
        }
        return node;
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

    return create_unary(operation, new_operand ? new_operand : operand, byte_size());
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

    return create_binary(new_left ? new_left : left, operation, new_right ? new_right : right, byte_size());
}

void ExpressionNodeBinary::serialize_sub(std::ostream &stream) const {
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

        case PLUS:
            stream << '+';
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

        case BANK_BYTE:
        case BITWISE_NOT:
        case HIGH_BYTE:
        case INTEGER:
        case LOW_BYTE:
        case MINUS:
        case SIZE:
        case VARIABLE:
            throw Exception("internal error: invalid binary operator");
    }

    stream << right << ')';
}


void ExpressionNodeUnary::serialize_sub(std::ostream &stream) const {
    switch (operation) {
        case BANK_BYTE:
            stream << '^';
            break;

        case BITWISE_NOT:
            stream << '~';
            break;

        case HIGH_BYTE:
            stream << '>';
            break;

        case LOW_BYTE:
            stream << '<';
            break;

        case MINUS:
            stream << '-';
            break;

        case ADD:
        case BITWISE_AND:
        case BITWISE_OR:
        case BITWISE_XOR:
        case DIVIDE:
        case INTEGER:
        case MODULO:
        case MULTIPLY:
        case PLUS:
        case SIZE:
        case SHIFT_LEFT:
        case SHIFT_RIGHT:
        case SUBTRACT:
        case VARIABLE:
            throw Exception("internal error: invalid unary operator");
    }

    stream << operand;
}