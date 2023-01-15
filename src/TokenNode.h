//
// Created by Dieter Baron on 15.01.23.
//

#ifndef KEYWORD_NODE_H
#define KEYWORD_NODE_H

#include "Node.h"
#include "Token.h"

class TokenNode: public Node {
public:
    explicit TokenNode(const Token& token);

    [[nodiscard]] Type type() const override {return node_type;}
    [[nodiscard]] symbol_t as_symbol() const {return symbol;}

private:
    Type node_type;
    symbol_t symbol;
};


#endif // KEYWORD_NODE_H
