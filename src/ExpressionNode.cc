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

    return std::make_shared<ExpressionNodeBinary>(left, it->second, right);
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
    tokenizer.add_punctuations({"+", "-", "~", "<", ">", "*", "/", /* mod */ "&", "^", "<<", ">>", "|", "(", ")"});

    if (!initialized) {
        token_ampersand = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("&"));
        token_caret = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("^"));
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
                {token_greater, HIGH_BYTE}
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
