//
// Created by Dieter Baron on 27.05.23.
//

#include "Visibility.h"

Token VisibilityHelper::token_local;
Token VisibilityHelper::token_global;
bool VisibilityHelper::initialized = false;

std::ostream& operator<<(std::ostream& stream, Visibility visibility) {
    switch  (visibility) {
        case Visibility::SCOPE:
            stream << "none";
            break;
        case Visibility::LOCAL:
            stream << "local";
            break;
        case Visibility::GLOBAL:
            stream << "global";
            break;
    }
    return stream;
}

void VisibilityHelper::setup(FileTokenizer& tokenizer) {
    if (!initialized) {
        token_local = Token(Token::DIRECTIVE, ".local");
        token_global = Token(Token::DIRECTIVE, ".global");
        initialized = true;
    }
    tokenizer.add_literal(token_local);
    tokenizer.add_literal(token_global);
}

std::optional<Visibility> VisibilityHelper::from_token(Token token) {
    if (token == token_local) {
        return Visibility::LOCAL;
    }
    else if (token == token_global) {
        return Visibility::GLOBAL;
    }
    else {
        return {};
    }
}
