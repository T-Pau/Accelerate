/*
Tokenizer.h -- Convert File into Stream of Tokens

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

#ifndef FILE_TOKENIZER_H
#define FILE_TOKENIZER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Tokenizer.h"
#include "Token.h"
#include "TokenGroup.h"
#include "Location.h"
#include "Path.h"

class FileTokenizer: public Tokenizer {
public:
    explicit FileTokenizer(std::shared_ptr<const Path> path = std::make_shared<const Path>(), bool use_preprocessor = true);
    void push(Symbol filename);

    [[nodiscard]] Location current_location() const override;
    [[nodiscard]] Symbol current_file() const {return current_source ? current_source->file() : Symbol();}
    Symbol find_file(Symbol file_name);

    void add_punctuations(const std::unordered_set<std::string>& names);
    void add_literal(const Token& token) { add_literal(token.get_type(), token.as_string());}
    void add_literal(Token::Type match, const std::string& name) {matcher.add(name.c_str(), match);}

protected:
    Token sub_next() override;
    [[nodiscard]] bool sub_ended() const override {return current_source == nullptr;}

private:
    class Source {
    public:
        Source(Symbol file, const std::vector<std::string>& lines) : file_(file), lines(lines) {}

        int next();
        void unget();

        [[nodiscard]] Symbol file() const {return file_;}
        [[nodiscard]] Location location() const { return {file(), line + 1, column, column}; }
        void expand_location(Location& location) const { location.end_column = column; }

        void reset_to(const Location& new_location);

    private:
        Symbol file_;
        const std::vector<std::string>& lines;
        size_t line = 0;
        size_t column = 0;
    };

    class MatcherNode {
    public:
        std::optional<Token::Type> match_type;
        bool match_in_word = false;
        std::unordered_map<char,MatcherNode> next;

        void add(const char* string, Token::Type type, bool match_in_word = false);
        std::optional<Token::Type> match(Source& source, std::string& name);
    };

    Token next_raw();

    Token parse_number(unsigned int base, Location location);
    Token parse_name(Token::Type type, Location location);
    Token parse_string(Location location);

    void preprocess(const std::vector<Token>& tokens);

    static int convert_digit(int c);
    static bool is_identifier_continuation(int c) {return is_identifier_start(c) || isdigit(c);}
    static bool is_identifier_start(int c) { return islower(c) || isupper(c) || c == '_'; }
    static bool is_identifier(const std::string& s);

    bool use_preprocessor;
    std::shared_ptr<const Path> path;

    static bool initialized;
    static Token token_include;

    MatcherNode matcher;

    std::vector<Source> sources;
    Source* current_source = nullptr;
    Location eof_location;

    bool last_was_newline = true;
};

#endif // FILE_TOKENIZER_H
