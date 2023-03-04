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
    [[nodiscard]] const Location& get_location() const override {return token.location;}

    [[nodiscard]] symbol_t as_symbol() const {return token.as_symbol();}

private:
    Type node_type;
    Token token;
};


#endif // KEYWORD_NODE_H
