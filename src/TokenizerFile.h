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

#ifndef TOKENIZER_FILE_H
#define TOKENIZER_FILE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Tokenizer.h"
#include "Token.h"
#include "TokenGroup.h"
#include "Location.h"

class TokenizerFile: public Tokenizer {
public:
    void push(const std::string& filename);

    [[nodiscard]] Location current_location() const;

    void add_punctuations(const std::unordered_set<std::string>& names);
    void add_literal(Token::Type match, const std::string& name) {matcher.add(name.c_str(), match);}

protected:
    Token sub_next() override;
    [[nodiscard]] bool sub_ended() const override {return current_source == nullptr;}

private:
    class Source {
    public:
        Source(symbol_t file, const std::vector<std::string>& lines) : file(file), lines(lines) {}

        int next();
        void unget();

        [[nodiscard]] Location location() const { return {file, line + 1, column, column}; }
        void expand_location(Location& location) const { location.end_column = column; }

        void reset_to(const Location& new_location);

    private:
        symbol_t file;
        const std::vector<std::string>& lines;
        size_t line = 0;
        size_t column = 0;
    };

    class MatcherNode {
    public:
        std::optional<Token::Type> match_type;
        std::unordered_map<char,MatcherNode> next;

        void add(const char* string, Token::Type type);
        std::optional<Token::Type> match(Source& source, std::string& name);
    };

    Token parse_number(unsigned int base, Location location);
    Token parse_name(Token::Type type, Location location);
    Token parse_string(Location location);

    static int convert_digit(int c);
    static bool isword(int c) { return islower(c) || isupper(c) || c == '_'; }

    MatcherNode matcher;

    std::vector<Source> sources;
    Source* current_source = nullptr;

    bool last_was_newline = false;
};

#endif // TOKENIZER_FILE_H
