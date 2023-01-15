//
// Created by Dieter Baron on 15.01.23.
//

#include "ParseException.h"
#include "TokenNode.h"

TokenNode::TokenNode(const Token &token) {
    switch (token.get_type()) {
        case Token::PUNCTUATION:
            node_type = PUNCTUATION;
            break;

        case Token::KEYWORD:
            node_type = KEYWORD;
            break;

        default:
            throw ParseException(token, "internal error: can't create TokenNode from %s", token.type_name());
    }

    location = token.location;
    symbol = token.as_symbol();
}
