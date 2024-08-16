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

#include "FunctionExpression.h"
#include "ParseException.h"
#include "Target.h"

const Token ExpressionParser::token_big_endian = Token{Token::NAME, "big_endian"};
const Token ExpressionParser::token_false = Token{Token::NAME, ".false"};
const Token ExpressionParser::token_little_endian = Token{Token::NAME, "little_endian"};
const Token ExpressionParser::token_mod = Token{Token::NAME, ".mod"};
const Token ExpressionParser::token_none = Token{Token::NAME, ".none"};
const Token ExpressionParser::token_string = Token{Token::NAME, "string"};
const Token ExpressionParser::token_true = Token{Token::NAME, ".true"};

std::unordered_map<Token, ExpressionParser::BinaryOperator> ExpressionParser::binary_operators;
std::unordered_map<Token, Expression::UnaryOperation> ExpressionParser::unary_operators;
bool ExpressionParser::initialized = false;

void ExpressionParser::initialize() {
    if (initialized) {
        return;
    }
    binary_operators = {
        {Token::double_pipe, BinaryOperator(Expression::BinaryOperation::LOGICAL_OR, 1)},

        {Token::double_ampersand, BinaryOperator(Expression::BinaryOperation::LOGICAL_AND, 2)},

        {Token::double_equals, BinaryOperator(Expression::BinaryOperation::EQUAL, 3)},
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
        {token_mod, BinaryOperator(Expression::BinaryOperation::MODULO, 5)},

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

    initialized = true;
}


ExpressionParser::Element ExpressionParser::next_element() {
    auto token = tokenizer.next();

    if (token.is_value() || token.is_string()) {
        return {Expression(token)};
    }
    else if (token.is_name()) {
        if (token == token_false) {
            return {Expression{token.location, false}};
        }
        else if (token == token_true) {
            return {Expression{token.location, true}};
        }
        else if (token == token_none) {
            return {Expression{token.location, Value{}}};
        }
        auto next_token = tokenizer.next();
        if (next_token == Token::paren_open) {
            return Element(token, FUNCTION_CALL);
        }
        else {
            tokenizer.unget(next_token);
            return Element(token);
        }
    }
    else if (token == Token::colon_minus || token == Token::colon_plus) {
        return {Expression(token), 0, UNNAMED_LABEL};
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
            case FUNCTION_CALL:
            case BINARY_OPERATOR:
            case PARENTHESIS_OPEN:
            case START:
            case UNARY_OPERATOR: {
                auto it = unary_operators.find(token);
                if (it != unary_operators.end()) {
                    return {token.location, it->second};
                }
                break;
            }

            case OPERAND:
            case UNNAMED_LABEL:
            case PARENTHESIS_CLOSED: {
                auto it = binary_operators.find(token);
                if (it != binary_operators.end()) {
                    return {token.location, it->second};
                }
                break;
            }

            case COMMA:
            case END:
                break;
        }
    }

    tokenizer.unget(token);
    return Element(token.location, END);
}


void ExpressionParser::setup(FileTokenizer &tokenizer) {
    initialize();
    tokenizer.add_punctuations({"(", ")", ":", ","});
    for (const auto& pair: binary_operators) {
        tokenizer.add_literal(pair.first);
    }
    for (const auto& pair: unary_operators) {
        tokenizer.add_literal(pair.first);
    }
    tokenizer.add_literal(token_false);
    tokenizer.add_literal(token_none);
    tokenizer.add_literal(token_true);
    FunctionExpression::setup(tokenizer);
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

                    case FUNCTION_CALL:
                    case OPERAND:
                    case UNNAMED_LABEL:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        shift(next);
                        break;

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
                        throw ParseException(next.location, "unexpected %s", next.description());

                    case OPERAND:
                    case UNNAMED_LABEL:
                        reduce_unary(next);
                        break;

                    case FUNCTION_CALL:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                        shift(next);
                        break;

                    case START:
                        // can't happen: next can never be argument_list or start
                        break;
                }
                break;
            }

            case OPERAND:
            case UNNAMED_LABEL: {
                switch (next.type) {
                    case FUNCTION_CALL:
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
                            if (stack.back().type == FUNCTION_CALL) {
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

                    case UNNAMED_LABEL:
                        tokenizer.unget(Token(Token::PUNCTUATION, next.location, next.node.as_variable()->variable()));
                        // fallthrough
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
                        else if (stack.back().type == FUNCTION_CALL) {
                            reduce_argument_list();
                            reduce_function_call();
                        }
                        else {
                            throw ParseException(next.location, "unmatched ')'");
                        }
                        break;

                    case START:
                        // can't happen: next can never be argument_list or start
                        break;
                }
                break;
            }

            case FUNCTION_CALL:
            case PARENTHESIS_OPEN:
            case START: {
                switch (next.type) {
                    case PARENTHESIS_CLOSED:
                        if (top.type == FUNCTION_CALL) {
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

                    case FUNCTION_CALL:
                    case OPERAND:
                    case PARENTHESIS_OPEN:
                    case UNARY_OPERATOR:
                    case UNNAMED_LABEL:
                        shift(next);
                        break;

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
        throw ParseException({top.location, next.location}, "internal error: invalid element types in reduce_unary");
    }
    top = Element(Expression({top.location, next.location}, top.operation.unary, next.node), 0);
}

void ExpressionParser::reduce_binary(int up_to_level) {
    while (top.is_operand() && !stack.empty()) {
        if (stack.back().is_unary_operator()) {
            top = Element(Expression({stack.back().location, top.location}, stack.back().operation.unary, top.node), 0);
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

        top = Element(Expression({left.location, top.location}, left.node, operation.operation.binary.operation, top.node), operation.level);

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
        auto encoding = parse_encoding();

        list.emplace_back(expression, encoding);

        auto token = tokenizer.next();

        if (!token || token.is_newline()) {
            break;
        }
        else if (token != Token::comma) {
            throw ParseException(token, "expected ':', ',', or newline");
        }
    }

    return Body(list);
}

std::optional<Encoder> ExpressionParser::parse_encoding() {
    IntegerEncoder::Type type;
    auto name_allowed = false;

    auto token = tokenizer.next();

    if (token == Token::colon) {
        token = tokenizer.next();
        if (token == Token::minus) {
            type = IntegerEncoder::SIGNED;
        }
        else if (token == Token::plus) {
            type = IntegerEncoder::UNSIGNED;
        }
        else {
            type = IntegerEncoder::UNSIGNED;
            name_allowed = true;
            tokenizer.unget(token);
        }
    }
    else if (token == Token::colon_minus) {
        type = IntegerEncoder::SIGNED;
    }
    else if (token == Token::colon_plus) {
        type = IntegerEncoder::UNSIGNED;
    }
    else {
        tokenizer.unget(token);
        return {};
    }

    token = tokenizer.next();

    if (token.is_unsigned()) {
        // TODO: check overflow
        return Encoder{IntegerEncoder{type, static_cast<size_t>(token.as_unsigned())}};
    }
    else if (token.is_name()) {
        if (!name_allowed) {
            throw ParseException(token, "expected integer");
        }
        auto size = std::optional<size_t>{};
        auto token2 = tokenizer.next();
        if (token2 == Token::paren_open) {
            token2 = tokenizer.expect(Token::VALUE);
            if (!token2.is_unsigned()) {
                throw ParseException(token2, "encoding size must be unsigned");
            }
            size = token2.as_unsigned();
            tokenizer.expect(Token::paren_close);
        }
        else {
            tokenizer.unget(token2);
        }

        if (token == token_big_endian) {
            return Encoder(IntegerEncoder(IntegerEncoder::UNSIGNED, size, IntegerEncoder::big_endian_byte_order));
        }
        else if (token == token_little_endian) {
            return Encoder(IntegerEncoder(IntegerEncoder::UNSIGNED, size, IntegerEncoder::little_endian_byte_order));
        }
        auto string_encoding = Target::current_target->default_string_encoding;
        if (token != token_string) {
            string_encoding = Target::current_target->string_encoding(token.as_symbol());
        }
        if (!string_encoding) {
            throw ParseException(token, "unknown string encoding '%s'", token.as_string().c_str());
        }
        return Encoder{string_encoding, size};
    }

    throw ParseException(token, "expected integer or name");
}

void ExpressionParser::reduce_argument_list() {
    auto previous = stack.back();
    previous.arguments.emplace_back(top.node);
    top = previous;
    stack.pop_back();
}

void ExpressionParser::reduce_function_call() {
    const auto name = top.node.as_variable();
    top = Element(Expression({top.location, tokenizer.current_location()}, name->variable(), top.arguments), 0);
}


const char *ExpressionParser::Element::description() const {
    switch (type) {
        case BINARY_OPERATOR:
            return "binary operator";

        case COMMA:
            return ",";

        case END:
            return "end of expression";

        case FUNCTION_CALL:
            return "function call";

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

        case UNNAMED_LABEL:
            return "unnamed label";

    }

    throw Exception("invalid element type %d", type);
}

ExpressionParser::Element::Element(const Location& location, ExpressionParser::BinaryOperator binary): type(BINARY_OPERATOR), level(binary.level), location(location) {
    operation.binary = binary;
}

ExpressionParser::Element::Element(const Location& location, Expression::UnaryOperation unary): type(UNARY_OPERATOR), level(0), location(location) {
    operation.unary = unary;
}
