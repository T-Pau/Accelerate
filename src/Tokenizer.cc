/*
Tokenizer.cc -- Convert File into Stream of Tokens

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

#include "Tokenizer.h"
#include "Util.h"

std::unordered_map<char, Token::Type> Tokenizer::single_character_tokens = {
        {'\n', Token::NEWLINE},
        {'#',  Token::HASH},
        {'(',  Token::PARENTHESIS_OPEN},
        {')',  Token::PARENTHESIS_CLOSE},
        {'*',  Token::STAR},
        {'+',  Token::PLUS},
        {',',  Token::COMMA},
        {'-',  Token::MINUS},
        {':',  Token::COLON},
        {'[',  Token::SQUARE_PARENTHESIS_OPEN},
        {']',  Token::SQUARE_PARENTHESIS_CLOSE},
        {'{',  Token::CURLY_PARENTHESIS_OPEN},
        {'}',  Token::CURLY_PARENTHESIS_CLOSE},
        {'<',  Token::LESS},
        {'>', Token::GREATER},
        {'=', Token::EQUAL}
};

void Tokenizer::push(const std::string& file_name, const std::vector<std::string>& lines) {
    sources.emplace_back(file_name, lines);
    current_source = &sources[sources.size() - 1];
}

Token Tokenizer::next() {
    if (current_source == nullptr) {
        return Token(Token::END);
    }

    while (true) {
        auto location = current_source->location();
        auto c = current_source->next();

        if (c == EOF) {
            current_source = nullptr;
            sources.pop_back();
            if (sources.empty()) {
                return Token(Token::END);
            }
            current_source = &sources[sources.size() - 1];
            continue;
        }

        auto it = single_character_tokens.find(static_cast<char>(c));
        if (it != single_character_tokens.end()) {
            // Not using expand_location() here to correctly handle \n.
            location.end_column += 1;
            return {it->second, location};
        }

        if (isspace(c)) {
            // skip whitespace
            continue;
        }
        else if (c == ';') {
            // skip comments
            while (current_source->next() != '\n') {}
            continue;
        }
        else if (c == '$') {
            return parse_number(16, location);
        }
        else if (c == '%') {
            return parse_number(2, location);
        }
        else if (isdigit(c)) {
            current_source->unget();
            return parse_number(10, location);
        }
        else if (c == '.') {
            return parse_name(Token::DIRECTIVE, location);
        }
        else if (isword(c)) {
            current_source->unget();
            return parse_name(Token::NAME, location);
        }
        else if (c == '"') {
            return parse_string(location);
        }
        else {
            return {Token::ERROR, location, string_format("illegal character '%c'", c)};
        }
    }
}

Token Tokenizer::parse_number(unsigned int base, Location location) {
    auto empty = true;
    uint64_t integer = 0;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();
        int digit = convert_digit(c);
        // TODO: real numbers
        if (digit < 0 || digit >= base) {
            current_source->unget();
            if (empty) {
                return {Token::ERROR, location, "empty number"};
            }
            return {Token::NUMBER, location, integer};
        }

        empty = false;
        integer = integer * base + digit;
    }
}

int Tokenizer::convert_digit(int c) {
    if (isdigit(c)) {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    else {
        return -1;
    }
}

Token Tokenizer::parse_name(Token::Type type, Location location) {
    std::string name;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();

        if (isword(c) || (!name.empty()) && isdigit(c)) {
            name += static_cast<char>(c);
        }
        else {
            current_source->unget();
            if (name.empty()) {
                return {Token::ERROR, location, "empty directive"};
            }
            return {type, location, name};
        }
    }

}

Token Tokenizer::parse_string(Location location) {
    std::string value;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();

        switch (c) {
            case '\n':
                return {Token::ERROR, location, "unterminated string"};

            case '"':
                current_source->expand_location(location);
                return {Token::STRING, location, value};

            case '\\': {
                current_source->expand_location(location);
                auto c = current_source->next();
                switch (c) {
                    case '\\':
                    case '\"':
                    case '\'':
                        value += static_cast<char>(c);
                        break;

                    case 'n':
                        value += '\n';
                        break;

                    case '\n':
                        return {Token::ERROR, location, "unterminated string"};

                    default:
                        current_source->expand_location(location);
                        // TODO: warning: invalid backslash escape
                        break;
                }
                break;
            }

            default:
                value += static_cast<char>(c);
                break;
        }
    }
}


int Tokenizer::Source::next() {
    if (line_number >= lines.size()) {
        return EOF;
    }

    if (column >= lines[line_number].size()) {
        line_number += 1;
        column = 0;
        return '\n';
    }

    auto c = lines[line_number][column];
    column += 1;
    return c;
}

void Tokenizer::Source::unget() {
    if (column == 0) {
        if (line_number > 0) {
            line_number -= 1;
            column = lines[line_number].size();
        }
    }
    else {
        column -= 1;
    }
}
