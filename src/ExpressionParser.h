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

#include "Tokenizer.h"
#include "ExpressionNode.h"

class ExpressionParser {
public:
    explicit ExpressionParser(Tokenizer& tokenizer) : tokenizer(tokenizer) {}

    static void setup(TokenizerFile& tokenizer);

    std::shared_ptr<ExpressionNode> parse() {top = Element({}, START); return do_parse();}
    std::shared_ptr<ExpressionNode> parse(const std::shared_ptr<ExpressionNode>& left) {top = Element(left, 0); return do_parse();}
    std::vector<std::shared_ptr<ExpressionNode>> parse_list();

private:
    class BinaryOperator {
    public:
        BinaryOperator(ExpressionNode::SubType opearation, int level): opeartion(opearation), level(level) {}

        ExpressionNode::SubType opeartion;
        int level;
    };

    enum ElementType {
        BINARY_OPERATOR,
        END,
        UNARY_OPERATOR,
        OPERAND,
        PARENTHESIS_CLOSED,
        PARENTHESIS_OPEN,
        START
    };

    class Element {
    public:
        Element(const std::shared_ptr<ExpressionNode>& node, int level): type(OPERAND), node(node), level(level), location(node->location) {}
        Element(Location location, ElementType type, ExpressionNode::SubType operation, int level): type(type), operation(operation), level(level), location(location) {}
        explicit Element(Location location, ElementType type): type(type), location(location) {}

        [[nodiscard]] const char* description() const;

        ElementType type;
        int level = 0;

        std::shared_ptr<ExpressionNode> node;
        ExpressionNode::SubType operation = ExpressionNode::INTEGER; // TODO: better value for not used

        Location location;
    };

    static void initialize();

    std::shared_ptr<ExpressionNode> do_parse();
    Element next_element();
    void reduce_binary(int up_to_level);
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
    static std::unordered_map<Token, ExpressionNode::SubType> unary_operators;
};


#endif // EXPRESSION_PARSER_H
