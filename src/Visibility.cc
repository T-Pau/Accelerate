//
// Created by Dieter Baron on 27.05.23.
//

#include "Visibility.h"

bool VisibilityHelper::initialized = false;
Token VisibilityHelper::token_local_directive;
Token VisibilityHelper::token_local_name;
Token VisibilityHelper::token_global_directive;
Token VisibilityHelper::token_global_name;

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

void VisibilityHelper::initialize() {
    if (!initialized) {
        token_global_directive = Token(Token::DIRECTIVE, ".global");
        token_global_name = Token(Token::NAME, "global");
        token_local_directive = Token(Token::DIRECTIVE, ".local");
        token_local_name = Token(Token::NAME, "local");
        initialized = true;
    }
}

void VisibilityHelper::setup(FileTokenizer& tokenizer, bool use_directives) {
    initialize();
    tokenizer.add_literal(token_local_directive);
    tokenizer.add_literal(token_global_directive);
}

std::optional<Visibility> VisibilityHelper::from_token(Token token) {
    if (token == token_local_directive || token == token_local_name) {
        return Visibility::LOCAL;
    }
    else if (token == token_global_directive || token == token_global_name) {
        return Visibility::GLOBAL;
    }
    else {
        return {};
    }
}
