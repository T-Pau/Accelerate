/*
ExpressionNode.cc -- Abstract Base Class of Expression Nodes

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

#include "ExpressionNode.h"

#include <utility>
#include "ParseException.h"

bool ExpressionNode::initialized;
Token ExpressionNode::token_ampersand;
Token ExpressionNode::token_caret;
Token ExpressionNode::token_comma;
Token ExpressionNode::token_double_greater;
Token ExpressionNode::token_double_less;
Token ExpressionNode::token_greater;
Token ExpressionNode::token_less;
Token ExpressionNode::token_minus;
Token ExpressionNode::token_paren_close;
Token ExpressionNode::token_paren_open;
Token ExpressionNode::token_pipe;
Token ExpressionNode::token_plus;
Token ExpressionNode::token_slash;
Token ExpressionNode::token_star;
Token ExpressionNode::token_tilde;

std::unordered_map<Token, ExpressionNode::SubType> ExpressionNode::unary_operators;
std::unordered_map<Token, ExpressionNode::SubType> ExpressionNode::additive_operators;
std::unordered_map<Token, ExpressionNode::SubType> ExpressionNode::multiplicative_operators;


std::shared_ptr<ExpressionNode> ExpressionNode::parse(Tokenizer &tokenizer) {
    auto left = parse_multiplicative_term(tokenizer);

    auto token = tokenizer.next();

    auto it = additive_operators.find(token);
    if (it == additive_operators.end()) {
        tokenizer.unget(token);
        return left;
    }

    auto right = parse_multiplicative_term(tokenizer);

    return evaluate(std::make_shared<ExpressionNodeBinary>(left, it->second, right), {});
}


std::shared_ptr<ExpressionNode> ExpressionNode::parse(Tokenizer& tokenizer, std::shared_ptr<ExpressionNode> left) {
    // TODO: check that it works for all binary operands

    while (true) {
        auto token = tokenizer.next();

        auto it = multiplicative_operators.find(token);
        if (it == multiplicative_operators.end()) {
            tokenizer.unget(token);
            return evaluate(left, {});
        }

        auto right = parse_unary_term(tokenizer);

        left = std::make_shared<ExpressionNodeBinary>(left, it->second, right);
    }
}


std::shared_ptr<ExpressionNode> ExpressionNode::parse_multiplicative_term(Tokenizer &tokenizer) {
    auto left = parse_unary_term(tokenizer);

    auto token = tokenizer.next();

    auto it = multiplicative_operators.find(token);
    if (it == multiplicative_operators.end()) {
        tokenizer.unget(token);
        return left;
    }

    auto right = parse_unary_term(tokenizer);

    return std::make_shared<ExpressionNodeBinary>(left, it->second, right);
}


std::shared_ptr<ExpressionNode> ExpressionNode::parse_unary_term(Tokenizer &tokenizer) {
    auto token = tokenizer.next();

    SubType operation;

    auto it = unary_operators.find(token);
    if (it == unary_operators.end()) {
        tokenizer.unget(token);
        operation = PLUS;
    }
    else {
        operation = it->second;
    }

    auto operand = parse_operand(tokenizer);

    if (operation == PLUS) {
        return operand;
    }
    else {
        return std::make_shared<ExpressionNodeUnary>(operation, operand);
    }
}


std::shared_ptr<ExpressionNode> ExpressionNode::parse_operand(Tokenizer &tokenizer) {
    auto token = tokenizer.next();

    if (token == token_paren_open) {
        auto expression = parse(tokenizer);
        token = tokenizer.next();
        if (token != token_paren_close) {
            throw ParseException(token, "expected ')'");
        }

        return expression;
    }

    switch (token.get_type()) {
        case Token::NAME:
            return std::make_shared<ExpressionNodeVariable>(token);

        case Token::INTEGER:
            return std::make_shared<ExpressionNodeInteger>(token);

        default:
            throw ParseException(token, "expected variable or number");
    }
}


void ExpressionNode::add_literals(TokenizerFile &tokenizer) {
    tokenizer.add_punctuations({"+", "-", "~", "<", ">", "*", "/", /* mod */ "&", "^", "<<", ">>", "|", "(", ")", ","});

    if (!initialized) {
        token_ampersand = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("&"));
        token_caret = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("^"));
        token_comma = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(","));
        token_double_greater = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(">>"));
        token_double_less = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("<<"));
        token_greater = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(">"));
        token_less = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("<"));
        token_minus = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("-"));
        token_paren_close = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(")"));
        token_paren_open = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("("));
        token_pipe = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("|"));
        token_plus = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("+"));
        token_slash = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("/"));
        token_star = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("*"));
        token_tilde = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("~"));

        unary_operators = {
                {token_plus, PLUS},
                {token_minus, MINUS},
                {token_caret, BANK_BYTE},
                {token_less, LOW_BYTE},
                {token_greater, HIGH_BYTE},
                {token_tilde, BITWISE_NOT}
        };

        additive_operators = {
                {token_plus, ADD},
                {token_minus, SUBTRACT},
                {token_pipe, BITWISE_OR}
        };

        multiplicative_operators = {
                {token_star, MULTIPLY},
                {token_slash, DIVIDE},
                {token_ampersand, BITWISE_AND},
                {token_caret, BITWISE_XOR},
                {token_double_less, SHIFT_LEFT},
                {token_double_greater, SHIFT_RIGHT}
        };
    }
}

std::vector<std::shared_ptr<ExpressionNode>> ExpressionNode::parse_list(Tokenizer &tokenizer) {
    std::vector<std::shared_ptr<ExpressionNode>> list;

    while (true) {
        list.emplace_back(parse(tokenizer));
        auto token = tokenizer.next();
        if (!token || token.is_newline()) {
            break;
        }
        else if (token != token_comma) {
            throw ParseException(token, "expected ',' or newline");
        }
    }

    return list;
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

    if (new_operand->subtype() == INTEGER) {
        auto value = std::dynamic_pointer_cast<ExpressionNodeInteger>(new_operand)->as_int();
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
        return std::make_shared<ExpressionNodeUnary>(operation, new_operand);
    }
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

    if (new_left && new_left->subtype() == INTEGER && new_right && new_right->subtype() == INTEGER) {
        auto left_value = std::dynamic_pointer_cast<ExpressionNodeInteger>(new_left)->as_int();
        auto right_value = std::dynamic_pointer_cast<ExpressionNodeInteger>(new_right)->as_int();
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
        return std::make_shared<ExpressionNodeBinary>(new_left ? new_left : left, operation, new_right ? new_right : right);
    }
}
