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

#include "ValueExpression.h"
#include "ParseException.h"
#include "VariableExpression.h"
#include "FunctionExpression.h"


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
std::unordered_map<Token, UnaryExpression::Operation> ExpressionParser::unary_operators;

void ExpressionParser::initialize() {
    if (initialized) {
        return;
    }

    token_ampersand = Token(Token::PUNCTUATION, "&");
    token_caret = Token(Token::PUNCTUATION, "^");
    token_comma = Token(Token::PUNCTUATION, ",");
    token_colon = Token(Token::PUNCTUATION, ":");
    token_double_greater = Token(Token::PUNCTUATION, ">>");
    token_double_less = Token(Token::PUNCTUATION, "<<");
    token_greater = Token(Token::PUNCTUATION, ">");
    token_less = Token(Token::PUNCTUATION, "<");
    token_minus = Token(Token::PUNCTUATION, "-");
    token_paren_close = Token(Token::PUNCTUATION, ")");
    token_paren_open = Token(Token::PUNCTUATION, "(");
    token_pipe = Token(Token::PUNCTUATION, "|");
    token_plus = Token(Token::PUNCTUATION, "+");
    token_slash = Token(Token::PUNCTUATION, "/");
    token_star = Token(Token::PUNCTUATION, "*");
    token_tilde = Token(Token::PUNCTUATION, "~");

    unary_operators = {
            {token_plus,    UnaryExpression::PLUS},
            {token_minus,   UnaryExpression::MINUS},
            {token_caret,   UnaryExpression::BANK_BYTE},
            {token_less,    UnaryExpression::LOW_BYTE},
            {token_greater, UnaryExpression::HIGH_BYTE},
            {token_tilde,   UnaryExpression::BITWISE_NOT}
    };

    binary_operators = {
            {token_plus, BinaryOperator(BinaryExpression::ADD, 1)},
            {token_minus, BinaryOperator(BinaryExpression::SUBTRACT, 1)},
            {token_pipe, BinaryOperator(BinaryExpression::BITWISE_OR, 1)},

            {token_star, BinaryOperator(BinaryExpression::MULTIPLY, 2)},
            {token_slash, BinaryOperator(BinaryExpression::DIVIDE, 2)},
            {token_ampersand, BinaryOperator(BinaryExpression::BITWISE_AND, 2)},
            {token_caret, BinaryOperator(BinaryExpression::BITWISE_XOR, 2)},
            {token_double_less, BinaryOperator(BinaryExpression::SHIFT_LEFT, 2)},
            {token_double_greater, BinaryOperator(BinaryExpression::SHIFT_RIGHT, 2)}
    };
}

ExpressionParser::Element ExpressionParser::next_element() {
    auto token = tokenizer.next();

    if (token.is_integer()) {
        return {std::make_shared<ValueExpression>(token), 0};
    }
    else if (token.is_name()) {
        return Element(token);
    }
    else if (token == token_comma) {
        return Element(token.location, COMMA);
    }
    else if (token == token_paren_close) {
        return Element(token.location, PARENTHESIS_CLOSED);
    }
    else if (token == token_paren_open) {
        return Element(token.location, PARENTHESIS_OPEN);
    }
    else {
        switch (top.type) {
            case ARGUMENT_LIST:
            case BINARY_OPERATOR:
            case PARENTHESIS_OPEN:
            case START: {
                auto it = unary_operators.find(token);
                if (it != unary_operators.end()) {
                    return {token.location, it->second};
                }
                break;
            }

            case NAME:
            case OPERAND:
            case PARENTHESIS_CLOSED: {
                auto it = binary_operators.find(token);
                if (it != binary_operators.end()) {
                    return {token.location, it->second};
                }
                break;
            }

            case COMMA:
            case END:
            case UNARY_OPERATOR:
                break;
        }
    }

    tokenizer.unget(token);
    return Element(token.location, END);
}


void ExpressionParser::setup(FileTokenizer &tokenizer) {
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
                    case COMMA:
                    case END:
                    case PARENTHESIS_CLOSED:
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case NAME:
                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        shift(next);
                        break;

                    case ARGUMENT_LIST:
                    case START:
                        // can't happen: next can never be argument_list or start
                        break;
                }
                break;
            }

            case COMMA:
            case END:
            case PARENTHESIS_CLOSED:
                // can't happen: these are never put on stack
                break;

            case UNARY_OPERATOR: {
                switch (next.type) {
                    case BINARY_OPERATOR:
                    case COMMA:
                    case END:
                    case PARENTHESIS_CLOSED:
                    case UNARY_OPERATOR:
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case NAME:
                    case OPERAND:
                        reduce_unary(next);
                        break;

                    case PARENTHESIS_OPEN:
                        shift(next);
                        break;

                    case ARGUMENT_LIST:
                    case START:
                        // can't happen: next can never be argument_list or start
                        break;
                }
                break;
            }

            case NAME:
                if (next.type == PARENTHESIS_OPEN) {
                    shift(Element(next.location, ARGUMENT_LIST));
                    break;
                }
                // fallthrough
            case OPERAND: {
                switch (next.type) {
                    case NAME:
                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case BINARY_OPERATOR:
                        reduce_binary(next.level);
                        shift(next);
                        break;

                    case COMMA: {
                        reduce_binary(0);
                        if (!stack.empty()) {
                            if (stack.back().type == ARGUMENT_LIST) {
                                reduce_argument_list();
                                break;
                            }
                            throw ParseException(stack.back().location, "unmatched '('");
                        }
                        auto token = token_comma;
                        token.location = next.location;
                        tokenizer.unget(token);
                        return top.node;
                    }

                    case END:
                        reduce_binary(0);
                        if (!stack.empty()) {
                            throw ParseException(stack.back().location, "unmatched '('");
                        }
                        return top.node;

                    case PARENTHESIS_CLOSED:
                        reduce_binary(0);
                        if (stack.empty()) {
                            auto token = token_paren_close;
                            token.location = next.location;
                            tokenizer.unget(token);
                            return top.node;
                        }
                        else if (stack.back().type == PARENTHESIS_OPEN) {
                            stack.pop_back();
                        }
                        else if (stack.back().type == ARGUMENT_LIST) {
                            reduce_argument_list();
                            reduce_function_call();
                        }
                        else {
                            throw ParseException(next.location, "unmatched ')'");
                        }
                        break;

                    case ARGUMENT_LIST:
                    case START:
                        // can't happen: next can never be argument_list or start
                        break;
                }
                break;
            }

            case ARGUMENT_LIST:
            case PARENTHESIS_OPEN:
            case START: {
                switch (next.type) {
                    case PARENTHESIS_CLOSED:
                        if (top.type == ARGUMENT_LIST) {
                            reduce_function_call();
                            break;
                        }
                        // fallthrough
                    case BINARY_OPERATOR:
                    case COMMA:
                    case END:
                        if (top.type == START && next.type == END) {
                            throw ParseException(next.location, "expected expression");
                        }
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case NAME:
                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        shift(next);
                        break;

                    case ARGUMENT_LIST:
                    case START:
                        // can't happen: next can never be argument_list or start
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
    top = Element(UnaryExpression::create(top.operation.unary, next.node), 0);
}

void ExpressionParser::reduce_binary(int up_to_level) {
    while (top.is_operand() && stack.size() >= 2) {
        const auto& operation = stack[stack.size() - 1];
        const auto& left = stack[stack.size() - 2];

        if (operation.type != BINARY_OPERATOR || operation.level < up_to_level || !left.is_operand()) {
            // TODO: error?
            break;
        }

        top = Element(BinaryExpression::create(left.node, operation.operation.binary.operation, top.node), operation.level);

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


std::unique_ptr<DataBodyElement> ExpressionParser::parse_list() {
    auto list = std::make_unique<DataBodyElement>();

    while (true) {
        auto expression = parse();
        auto encoding = std::optional<Encoding>();

        auto token = tokenizer.next();

        if (token == token_colon) {
            encoding = parse_encoding();
            token = tokenizer.next();
        }

        list->append(expression, encoding);

        if (!token || token.is_newline()) {
            break;
        }
        else if (token != token_comma) {
            throw ParseException(token, "expected ':', ',', or newline");
        }
    }

    return list;
}

Encoding ExpressionParser::parse_encoding() {
    auto token = tokenizer.next();

    auto type = Encoding::UNSIGNED;

    if (token == token_minus) {
        type = Encoding::SIGNED;
        token = tokenizer.next();
    }

    if (token.is_unsigned()) {
        return {type, token.as_unsigned()};
    }

    throw ParseException(token, "expected integer");
}

void ExpressionParser::reduce_argument_list() {
    auto previous = stack.back();
    previous.arguments.emplace_back(top.node);
    top = previous;
    stack.pop_back();
}

void ExpressionParser::reduce_function_call() {
    auto previous = stack.back();
    auto name = std::dynamic_pointer_cast<VariableExpression>(previous.node)->variable();
    top = Element(std::make_shared<FunctionExpression>(name, top.arguments), 0);
    stack.pop_back();
}


const char *ExpressionParser::Element::description() const {
    switch (type) {
        case ARGUMENT_LIST:
            return "argument list";

        case BINARY_OPERATOR:
            return "binary operator";

        case COMMA:
            return ",";

        case END:
            return "end of expression";

        case NAME:
            return "name";

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

ExpressionParser::Element::Element(Location location, ExpressionParser::BinaryOperator binary): type(BINARY_OPERATOR), level(binary.level), location(location) {
    operation.binary = binary;
}

ExpressionParser::Element::Element(Location location, UnaryExpression::Operation unary): type(UNARY_OPERATOR), level(0), location(location) {
    operation.unary = unary;
}
