/*
ExpressionParser.cc -- 

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

#include "ExpressionParser.h"
#include "ParseException.h"


bool ExpressionParser::initialized;
Token ExpressionParser::token_ampersand;
Token ExpressionParser::token_caret;
Token ExpressionParser::token_comma;
Token ExpressionParser::token_colon;
Token ExpressionParser::token_double_greater;
Token ExpressionParser::token_double_less;
Token ExpressionParser::token_greater;
Token ExpressionParser::token_less;
Token ExpressionParser::token_minus;
Token ExpressionParser::token_paren_close;
Token ExpressionParser::token_paren_open;
Token ExpressionParser::token_pipe;
Token ExpressionParser::token_plus;
Token ExpressionParser::token_slash;
Token ExpressionParser::token_star;
Token ExpressionParser::token_tilde;

std::unordered_map<Token, ExpressionParser::BinaryOperator> ExpressionParser::binary_operators;
std::unordered_map<Token, Expression::SubType> ExpressionParser::unary_operators;

void ExpressionParser::initialize() {
    if (initialized) {
        return;
    }

    token_ampersand = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("&"));
    token_caret = Token(Token::PUNCTUATION, {}, SymbolTable::global.add("^"));
    token_comma = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(","));
    token_colon = Token(Token::PUNCTUATION, {}, SymbolTable::global.add(":"));
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
            {token_plus,    Expression::PLUS},
            {token_minus,   Expression::MINUS},
            {token_caret,   Expression::BANK_BYTE},
            {token_less,    Expression::LOW_BYTE},
            {token_greater, Expression::HIGH_BYTE},
            {token_tilde,   Expression::BITWISE_NOT}
    };

    binary_operators = {
            {token_colon, BinaryOperator(Expression::SIZE, 0)},

            {token_plus, BinaryOperator(Expression::ADD, 1)},
            {token_minus, BinaryOperator(Expression::SUBTRACT, 1)},
            {token_pipe, BinaryOperator(Expression::BITWISE_OR, 1)},

            {token_star, BinaryOperator(Expression::MULTIPLY, 2)},
            {token_slash, BinaryOperator(Expression::DIVIDE, 2)},
            {token_ampersand, BinaryOperator(Expression::BITWISE_AND, 2)},
            {token_caret, BinaryOperator(Expression::BITWISE_XOR, 2)},
            {token_double_less, BinaryOperator(Expression::SHIFT_LEFT, 2)},
            {token_double_greater, BinaryOperator(Expression::SHIFT_RIGHT, 2)}
    };
}

ExpressionParser::Element ExpressionParser::next_element() {
    auto token = tokenizer.next();

    if (token.is_integer()) {
        return {std::make_shared<IntegerExpression>(token), 0};
    }
    else if (token.is_name()) {
        return {std::make_shared<VariableExpression>(token), 0};
    }
    else if (token == token_paren_close) {
        return Element(token.location, PARENTHESIS_CLOSED);
    }
    else if (token == token_paren_open) {
        return Element(token.location, PARENTHESIS_OPEN);
    }
    else {
        switch (top.type) {
            case BINARY_OPERATOR:
            case PARENTHESIS_OPEN:
            case START: {
                auto it = unary_operators.find(token);
                if (it != unary_operators.end()) {
                    return {token.location, UNARY_OPERATOR, it->second, 0};
                }
                break;
            }

            case OPERAND:
            case PARENTHESIS_CLOSED: {
                auto it = binary_operators.find(token);
                if (it != binary_operators.end()) {
                    return {token.location, BINARY_OPERATOR, it->second.opeartion, it->second.level};
                }
                break;
            }

            case END:
            case UNARY_OPERATOR:
                break;
        }
    }

    tokenizer.unget(token);
    return Element(token.location, END);
}


void ExpressionParser::setup(TokenizerFile &tokenizer) {
    tokenizer.add_punctuations({"+", "-", "~", "<", ">", "*", "/", /* mod */ "&", "^", "<<", ">>", "|", "(", ")", ":", ","});
    initialize();
}

std::shared_ptr<Expression> ExpressionParser::do_parse() {
    stack.clear();

    while (true) {
        auto next = next_element();

        switch (top.type) {
            case BINARY_OPERATOR: {
                switch (next.type) {
                    case BINARY_OPERATOR:
                    case END:
                    case PARENTHESIS_CLOSED:
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        shift(next);
                        break;

                    case START:
                        // can't happen: next can never be start
                        break;
                }
                break;
            }

            case END:
            case PARENTHESIS_CLOSED:
                // can't happen: these are never put on stack
                break;

            case UNARY_OPERATOR: {
                switch (next.type) {
                    case BINARY_OPERATOR:
                    case END:
                    case PARENTHESIS_CLOSED:
                    case UNARY_OPERATOR:
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case OPERAND:
                        reduce_unary(next);
                        break;

                    case PARENTHESIS_OPEN:
                        shift(next);
                        break;

                    case START:
                        // can't happen: next can never be start
                        break;
                }
                break;
            }

            case OPERAND: {
                switch (next.type) {
                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case BINARY_OPERATOR:
                        reduce_binary(next.level);
                        shift(next);
                        break;

                    case END:
                        reduce_binary(std::numeric_limits<int>::max());
                        if (!stack.empty()) {
                            throw ParseException(stack.back().location, "unmatched '('");
                        }
                        return top.node;

                    case PARENTHESIS_CLOSED:
                        reduce_binary(std::numeric_limits<int>::max());
                        if (stack.empty()) {
                            auto token = token_paren_close;
                            token.location = next.location;
                            tokenizer.unget(token);
                            return top.node;
                        }
                        else if (stack.back().type == PARENTHESIS_OPEN) {
                            stack.pop_back();
                        }
                        else {
                            throw ParseException(next.location, "unmatched ')'");
                        }
                        break;

                    case START:
                        // can't happen: next can never be start
                        break;
                }
                break;
            }

            case PARENTHESIS_OPEN:
            case START: {
                switch (next.type) {
                    case BINARY_OPERATOR:
                    case END:
                    case PARENTHESIS_CLOSED:
                        if (top.type == START && next.type == END) {
                            throw ParseException(next.location, "expected expression");
                        }
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        shift(next);
                        break;

                    case START:
                        // can't happen: next can never be start
                        break;
                }
                break;
            }
        }
    }
}

void ExpressionParser::reduce_unary(const ExpressionParser::Element& next) {
    if (top.type != UNARY_OPERATOR || next.type != OPERAND) {
        throw Exception("internal error: invalid element types in reduce_unary");
    }
    top = Element(Expression::create_unary(top.operation, next.node, 0), 0);
}

void ExpressionParser::reduce_binary(int up_to_level) {
    while (top.type == OPERAND && stack.size() >= 2) {
        const auto& operation = stack[stack.size() - 1];
        const auto& left = stack[stack.size() - 2];

        if (operation.type != BINARY_OPERATOR || operation.level > up_to_level || left.type != OPERAND) {
            // TODO: error?
            break;
        }

        if (operation.operation == Expression::SIZE) {
            if (!top.node->has_value()) {
                throw ParseException(top.node->location, "size not a constant expression");
            }
            left.node->set_byte_size(top.node->value());
            top = left;
        }
        else {
            top = Element(Expression::create_binary(left.node, operation.operation, top.node, 0), operation.level);
        }

        stack.pop_back();
        stack.pop_back();
    }
}

void ExpressionParser::shift(ExpressionParser::Element next) {
    if (top.type != START) {
        stack.emplace_back(top);
    }
    top = std::move(next);
}


ExpressionList ExpressionParser::parse_list() {
    ExpressionList list;

    while (true) {
        list.expressions.emplace_back(parse());
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


const char *ExpressionParser::Element::description() const {
    switch (type) {
        case BINARY_OPERATOR:
            return "binary operator";

        case END:
            return "end of expression";

        case UNARY_OPERATOR:
            return "unary operator";

        case OPERAND:
            return "operand";

        case PARENTHESIS_CLOSED:
            return "')'";

        case PARENTHESIS_OPEN:
            return "'('";

        case START:
            return "beginning of expression";
    }
}
