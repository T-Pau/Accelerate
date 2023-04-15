/*
Token.cc -- 

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

#include "Token.h"

const std::string Token::empty_string;

const char* Token::type_name(Type type) {
    switch (type) {
        case PREPROCESSOR:
            return "preprocessor directive";

        case PUNCTUATION:
            return "punctuation";

        case DIRECTIVE:
            return "directive";

        case END:
            return "end of file";

        case NAME:
            return "name";

        case NEWLINE:
            return "newline";

        case VALUE:
            return "value";

        case STRING:
            return "string";

        case KEYWORD:
            return "keyword";

        case INSTRUCTION:
            return "instruction";
    }
}

bool Token::operator==(const Token &other) const {
    if (type != other.type) {
        return false;
    }
    switch (type) {
        case NAME:
        case DIRECTIVE:
        case KEYWORD:
        case PREPROCESSOR:
        case PUNCTUATION:
        case INSTRUCTION:
        case STRING:
            return symbol == other.symbol;

        case VALUE:
            return value == other.value;

        case END:
        case NEWLINE:
            return true;
    }
}

const std::string &Token::as_string() const {
    switch (type) {
        case NAME:
        case DIRECTIVE:
        case PREPROCESSOR:
        case PUNCTUATION:
        case STRING:
        case KEYWORD:
        case INSTRUCTION:
            return symbol.str();

        case VALUE:
        case END:
        case NEWLINE:
            return empty_string;
    }
}

Symbol Token::as_symbol() const {
    switch (type) {
        case NAME:
        case DIRECTIVE:
        case PREPROCESSOR:
        case PUNCTUATION:
        case STRING:
        case KEYWORD:
        case INSTRUCTION:
            return symbol;

        case VALUE:
        case END:
        case NEWLINE:
            return {};
    }
}

/*Token& Token::operator=(const Token& other) {
    type = other.type;
    switch (type) {
        case DIRECTIVE:
        case PUNCTUATION:
        case NAME:
        case PREPROCESSOR:
        case STRING:
        case KEYWORD:
        case INSTRUCTION:
            symbol = other.symbol;
            break;

        case VALUE:
            value = other.value;
            break;

        case END:
        case NEWLINE:
            break;
    }

    return *this;
}*/
