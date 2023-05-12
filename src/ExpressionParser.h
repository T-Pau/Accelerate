/*
ExpressionParser.h -- 

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

#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H


#include <utility>

#include "BinaryExpression.h"
#include "Tokenizer.h"
#include "UnaryExpression.h"
#include "Encoding.h"
#include "DataBody.h"
#include "VariableExpression.h"

class ExpressionParser {
public:
    explicit ExpressionParser(Tokenizer& tokenizer) : tokenizer(tokenizer) {}

    static void setup(FileTokenizer& tokenizer);

    Expression parse() { top = Element({}, START); return do_parse();}
    Expression parse(Expression& left) { top = Element(left, 0); return do_parse();}
    Body parse_list();

private:
    class BinaryOperator {
    public:
        BinaryOperator(Expression::BinaryOperation operation, int level): operation(operation), level(level) {}

        Expression::BinaryOperation operation;
        int level;
    };

    enum ElementType {
        ARGUMENT_LIST,
        BINARY_OPERATOR,
        COMMA,
        END,
        NAME,
        OPERAND,
        PARENTHESIS_CLOSED,
        PARENTHESIS_OPEN,
        START,
        UNARY_OPERATOR
    };

    class Element {
    public:
        explicit Element(const Token& token): type(NAME), node(std::make_shared<VariableExpression>(token.as_symbol())), level(0), location(token.location) {}
        Element(const Expression& node, int level): type(OPERAND), node(node), level(level), location(node.location()) {}
        Element(Location location, BinaryOperator binary);
        Element(Location location, Expression::UnaryOperation unary);
        explicit Element(Location location, ElementType type): type(type), location(location) {}

        [[nodiscard]] const char* description() const;
        [[nodiscard]] bool is_operand() const {return type == OPERAND || type == NAME;}

        ElementType type;
        int level = 0;

        Expression node;
        std::vector<Expression> arguments;
        union {
            int none;
            BinaryOperator binary;
            Expression::UnaryOperation unary;
        } operation = {0};

        Location location;
    };

    static void initialize();

    Expression do_parse();
    Element next_element();
    Encoding parse_encoding();
    void reduce_argument_list();
    void reduce_binary(int up_to_level);
    void reduce_function_call();
    void reduce_unary(const Element& next);
    void shift(Element next);

    Tokenizer& tokenizer;

    Element top = Element({}, START);
    std::vector<Element> stack;

    static bool initialized;
    static Token token_ampersand;
    static Token token_caret;
    static Token token_colon;
    static Token token_comma;
    static Token token_double_greater;
    static Token token_double_less;
    static Token token_greater;
    static Token token_less;
    static Token token_minus;
    static Token token_paren_close;
    static Token token_paren_open;
    static Token token_pipe;
    static Token token_plus;
    static Token token_slash;
    static Token token_star;
    static Token token_tilde;

    static std::unordered_map<Token, BinaryOperator> binary_operators;
    static std::unordered_map<Token, Expression::UnaryOperation> unary_operators;
};


#endif // EXPRESSION_PARSER_H
