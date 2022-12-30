/*
Tokenizer.h -- Convert File into Stream of Tokens

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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Token.h"
#include "Location.h"

class Tokenizer {
public:
    void push(const std::string& filename, const std::vector<std::string>& lines);

    Token next();

private:
    class Source {
    public:
        Source(std::string file_name, const std::vector<std::string>& lines) : file_name(std::move(file_name)), lines(lines) {}

        int next();
        void unget();

        [[nodiscard]] Location location() const { return {file_name, line_number, column, column}; }
        void expand_location(Location& location) const { location.end_column = column; }

    private:
        const std::string file_name;
        const std::vector<std::string>& lines;
        size_t line_number = 0;
        size_t column = 0;
    };

    Token parse_number(unsigned int base, Location location);
    Token parse_name(Token::Type type, Location location);
    Token parse_string(Location location);

    static int convert_digit(int c);
    static bool isword(int c) { return islower(c) || isupper(c) || c == '_'; }

    static std::unordered_map<char, Token::Type> single_character_tokens;

    std::vector<Source> sources;
    Source* current_source = nullptr;
};

#endif // TOKENIZER_H
