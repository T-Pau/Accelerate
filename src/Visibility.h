//
// Created by Dieter Baron on 27.05.23.
//

#ifndef VISIBILITY_H
#define VISIBILITY_H

#include <iostream>

#include "FileTokenizer.h"
#include "Token.h"

enum class Visibility {
    SCOPE,
    PRIVATE,
    PUBLIC
};

class VisibilityHelper {
  public:
    static std::optional<Visibility> from_token(Token token);
    static Token token_public_directive;
    static Token token_public_name;
    static Token token_private_directive;
    static Token token_private_name;

    static void initialize();
    static void setup(FileTokenizer& tokenizer, bool use_directives);

  private:
    static bool initialized;
};

std::ostream& operator<<(std::ostream& stream, Visibility visibility);

#endif // VISIBILITY_H
