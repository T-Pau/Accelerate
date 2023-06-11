//
// Created by Dieter Baron on 27.05.23.
//

#include "Visibility.h"

#define PRIVATE_LITERAL "private"
#define PUBLIC_LITERAL "public"

bool VisibilityHelper::initialized = false;
Token VisibilityHelper::token_private_directive;
Token VisibilityHelper::token_private_name;
Token VisibilityHelper::token_public_directive;
Token VisibilityHelper::token_public_name;

std::ostream& operator<<(std::ostream& stream, Visibility visibility) {
    switch  (visibility) {
        case Visibility::SCOPE:
            stream << "none";
            break;
        case Visibility::PUBLIC:
            stream << PUBLIC_LITERAL;
            break;
        case Visibility::PRIVATE:
            stream << PRIVATE_LITERAL;
            break;
    }
    return stream;
}

void VisibilityHelper::initialize() {
    if (!initialized) {
        token_public_directive = Token(Token::DIRECTIVE, PUBLIC_LITERAL);
        token_public_name = Token(Token::NAME, PUBLIC_LITERAL);
        token_private_directive = Token(Token::DIRECTIVE, PRIVATE_LITERAL);
        token_private_name = Token(Token::NAME, PRIVATE_LITERAL);
        initialized = true;
    }
}

void VisibilityHelper::setup(FileTokenizer& tokenizer, bool use_directives) {
    initialize();
}

std::optional<Visibility> VisibilityHelper::from_token(Token token) {
    if (token == token_private_directive || token == token_private_name) {
        return Visibility::PRIVATE;
    }
    else if (token == token_public_directive || token == token_public_name) {
        return Visibility::PUBLIC;
    }
    else {
        return {};
    }
}
