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

std::unordered_map<Token, ExpressionParser::BinaryOperator> ExpressionParser::binary_operators;

std::unordered_map<Token, Expression::UnaryOperation> ExpressionParser::unary_operators;

void ExpressionParser::initialize() {
    binary_operators = {
        {Token::double_pipe, BinaryOperator(Expression::BinaryOperation::LOGICAL_OR, 1)},

        {Token::double_ampersand, BinaryOperator(Expression::BinaryOperation::LOGICAL_AND, 2)},

        {Token::equals, BinaryOperator(Expression::BinaryOperation::EQUAL, 3)},
        {Token::greater, BinaryOperator(Expression::BinaryOperation::GREATER, 3)},
        {Token::greater_equals, BinaryOperator(Expression::BinaryOperation::GREATER_EQUAL, 3)},
        {Token::less, BinaryOperator(Expression::BinaryOperation::LESS, 3)},
        {Token::less_equals, BinaryOperator(Expression::BinaryOperation::LESS_EQUAL, 3)},
        {Token::exclaim_equals, BinaryOperator(Expression::BinaryOperation::NOT_EQUAL, 3)},

        {Token::plus, BinaryOperator(Expression::BinaryOperation::ADD, 4)},
        {Token::minus, BinaryOperator(Expression::BinaryOperation::SUBTRACT, 4)},
        {Token::pipe, BinaryOperator(Expression::BinaryOperation::BITWISE_OR, 4)},
        {Token::caret, BinaryOperator(Expression::BinaryOperation::BITWISE_XOR, 4)},

        {Token::star, BinaryOperator(Expression::BinaryOperation::MULTIPLY, 5)},
        {Token::slash, BinaryOperator(Expression::BinaryOperation::DIVIDE, 5)},
        {Token::ampersand, BinaryOperator(Expression::BinaryOperation::BITWISE_AND, 5)},

        {Token::double_less, BinaryOperator(Expression::BinaryOperation::SHIFT_LEFT, 6)},
        {Token::double_greater, BinaryOperator(Expression::BinaryOperation::SHIFT_RIGHT, 6)}
    };

    unary_operators = {
        {Token::exclaim, Expression::UnaryOperation::NOT},
        {Token::plus, Expression::UnaryOperation::PLUS},
        {Token::minus, Expression::UnaryOperation::MINUS},
        {Token::caret, Expression::UnaryOperation::BANK_BYTE},
        {Token::less, Expression::UnaryOperation::LOW_BYTE},
        {Token::greater, Expression::UnaryOperation::HIGH_BYTE},
        {Token::tilde, Expression::UnaryOperation::BITWISE_NOT}
    };
}


ExpressionParser::Element ExpressionParser::next_element() {
    auto token = tokenizer.next();

    if (token.is_integer()) {
        return {Expression(token), 0};
    }
    else if (token.is_name()) {
        return Element(token);
    }
    else if (token == Token::colon_minus || token == Token::colon_plus) {
        return {Expression(token), 0};
    }
    else if (token == Token::comma) {
        return Element(token.location, COMMA);
    }
    else if (token == Token::paren_close) {
        return Element(token.location, PARENTHESIS_CLOSED);
    }
    else if (token == Token::paren_open) {
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

Expression ExpressionParser::do_parse() {
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
                        auto token = Token::comma;
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
                            auto token = Token::paren_close;
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
    if (top.type != UNARY_OPERATOR || !next.is_operand()) {
        throw Exception("internal error: invalid element types in reduce_unary");
    }
    top = Element(Expression(top.operation.unary, next.node), 0);
}

void ExpressionParser::reduce_binary(int up_to_level) {
    while (top.is_operand() && !stack.empty()) {
        if (stack.back().is_unary_operator()) {
            top = Element(Expression(stack.back().operation.unary, top.node), 0);
            stack.pop_back();
        }
        if (top.is_operand() && stack.size() < 2) {
            break;
        }
        const auto& operation = stack[stack.size() - 1];
        const auto& left = stack[stack.size() - 2];

        if (operation.type != BINARY_OPERATOR || operation.level < up_to_level || !left.is_operand()) {
            // TODO: error?
            break;
        }

        top = Element(Expression(left.node, operation.operation.binary.operation, top.node), operation.level);

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


Body ExpressionParser::parse_list() {
    auto list = std::vector<DataBodyElement>();

    while (true) {
        auto expression = parse();
        auto encoding = std::optional<Encoding>();

        auto token = tokenizer.next();

        if (token == Token::colon || token == Token::colon_minus || token == Token::colon_plus) {
            encoding = parse_encoding(token);
            token = tokenizer.next();
        }

        list.emplace_back(expression, encoding);

        if (!token || token.is_newline()) {
            break;
        }
        else if (token != Token::comma) {
            throw ParseException(token, "expected ':', ',', or newline");
        }
    }

    return Body(list);
}

Encoding ExpressionParser::parse_encoding(Token token) {
    Encoding::Type type;

    if (token == Token::colon) {
        token = tokenizer.next();
        if (token == Token::minus) {
            type = Encoding::SIGNED;
        }
        else if (token == Token::plus) {
            type = Encoding::UNSIGNED;
        }
        else {
            type = Encoding::UNSIGNED;
            tokenizer.unget(token);
        }
    }
    else if (token == Token::colon_minus) {
        type = Encoding::SIGNED;
    }
    else if (token == Token::colon_plus) {
        type = Encoding::UNSIGNED;
    }

    token = tokenizer.next();

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
    auto name = previous.node.as_variable();
    top = Element(Expression(name->variable(), top.arguments), 0);
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

ExpressionParser::Element::Element(Location location, Expression::UnaryOperation unary): type(UNARY_OPERATOR), level(0), location(location) {
    operation.unary = unary;
}
