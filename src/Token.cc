/*
Token.cc -- 

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "Token.h"

const std::string Token::empty_string;

const char* Token::type_name(Type type) {
    switch (type) {
        case COLON:
            return "':'";

        case COMMA:
            return "','";

        case CURLY_PARENTHESIS_CLOSE:
            return "'}'";

        case CURLY_PARENTHESIS_OPEN:
            return "'{'";

        case DIRECTIVE:
            return "directive";

        case END:
            return "end of file";

        case EQUAL:
            return "'='";

        case ERROR:
            return "error";

        case GREATER:
            return "'>'";

        case HASH:
            return "'#'";

        case LESS:
            return "'<'";

        case MINUS:
            return "'-'";

        case NAME:
            return "name";

        case NEWLINE:
            return "newline";

        case NUMBER:
            return "number";

        case PARENTHESIS_CLOSE:
            return "')'";

        case PARENTHESIS_OPEN:
            return "'('";

        case PLUS:
            return "'+'";

        case SQUARE_PARENTHESIS_CLOSE:
            return "']'";

        case SQUARE_PARENTHESIS_OPEN:
            return "'['";

        case STAR:
            return "'*'";

        case STRING:
            return "string";
    }
}

bool Token::operator==(const Token &other) const {
    if (type != other.type) {
        return false;
    }
    switch (type) {
        case NAME:
        case DIRECTIVE:
            return value.symbol == other.value.symbol;

        case NUMBER:
            return value.integer == other.value.integer;

        case STRING:
            return string == other.string;

        default:
            return true;
    }
}

const std::string &Token::as_string() const {
    switch (type) {
        case NAME:
        case DIRECTIVE:
            return SymbolTable::global[value.symbol];

        case STRING:
            return string;

        default:
            return empty_string;
    }
}

Token::Token(Token::Type type, Location location, const std::string& name): type(type), location(std::move(location)) {
    switch (type) {
        case DIRECTIVE:
        case NAME:
            value.symbol = SymbolTable::global.add(name);
            break;

        case STRING:
            string = name;
            break;

        default:
            break;
    }
}
