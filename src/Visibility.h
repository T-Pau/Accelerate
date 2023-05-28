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
    LOCAL,
    GLOBAL
};

class VisibilityHelper {
  public:
    static std::optional<Visibility> from_token(Token token);
    static Token token_local;
    static Token token_global;

    static void setup(FileTokenizer& tokenizer);

  private:
    static bool initialized;
};

std::ostream& operator<<(std::ostream& stream, Visibility visibility);

#endif // VISIBILITY_H
