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
#include "Location.h"
#include "Path.h"

class FileTokenizer: public Tokenizer {
public:
    explicit FileTokenizer(const Path& path = Path::empty_path, bool use_preprocessor = true, const std::unordered_set<Symbol>& defines = {});
    void push(Symbol filename);

    [[nodiscard]] Location current_location() const override;
    [[nodiscard]] Symbol current_file() const {return current_source ? current_source->file() : Symbol();}
    Symbol find_file(Symbol file_name);

    void define(Symbol name) {defines.insert(name);}
    void define(const std::unordered_set<Symbol>& defines);
    void undefine(Symbol name) {defines.erase(name);}

    void add_punctuations(const std::unordered_set<std::string>& names);
    void add_literal(const Token& token) { add_literal(token.get_type(), token.as_string());}
    void add_literal(Token::Type match, const std::string& name, const std::string& suffix_characters = "");

    std::unordered_set<Symbol> defines;

protected:
    Token sub_next() override;
    [[nodiscard]] bool sub_ended() const override {return current_source == nullptr;}

private:
    class PreState {
    public:
        explicit PreState(bool condition): processing{condition}, skip_rest{condition} {}

        void process_else();
        void process_else_if(bool condition);

        [[nodiscard]] bool skipping_rest() const {return skip_rest;}
        operator bool() const {return processing;}

    private:
        bool processing;
        bool skip_rest;
        bool else_seen{false};
    };
    class Source {
    public:
        Source(Symbol file, const std::vector<std::string>& lines) : file_(file), lines(lines) {}

        int next();
        void unget();

        [[nodiscard]] Symbol file() const {return file_;}
        [[nodiscard]] Location location() const { return {file(), line + 1, column, column}; }
        void expand_location(Location& location) const { location.end_column = column; }

        void reset_to(const Location& new_location);

        std::vector<PreState> pre_states;

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
        std::unordered_map<char,std::unique_ptr<MatcherNode>> next;
        std::unordered_set<char> suffix_characters;

        void add(const char* string, Token::Type type, const std::unordered_set<char>& suffix_characters = {}, bool match_in_word = false);
        std::optional<Token::Type> match(Source& source, std::string& name);

      private:
        [[nodiscard]] bool conflicts(const std::unordered_set<char>& new_suffix) const;
    };

    class PreprocessorDirective {
    public:
        PreprocessorDirective(std::optional<size_t> min_arguments, std::optional<size_t> max_arguments, std::vector<TokenGroup> argument_type,void (FileTokenizer::*handler)(const Token&, const std::vector<Token>&)): min_arguments{min_arguments}, max_arguments{max_arguments}, argument_type{std::move(argument_type)}, handler{handler} {}

        void operator()(FileTokenizer& tokenizer, const Token& directive, const std::vector<Token>& arguments) const;

    private:
        std::optional<size_t> min_arguments;
        std::optional<size_t> max_arguments;
        std::vector<TokenGroup> argument_type;
        void (FileTokenizer::*handler)(const Token&, const std::vector<Token>&);
    };

    Token next_raw();

    [[nodiscard]] bool pre_is_procesing() const;

    Token parse_base64(Location location);
    Token parse_name(Token::Type type, Location location);
    Token parse_number(unsigned int base, Location location);
    Token parse_string(Location location);

    void preprocess(const Token& directive, const std::vector<Token>& tokens);
    void preprocess_define(const Token& directive, const std::vector<Token>& tokens);
    void preprocess_include(const Token& directive, const std::vector<Token>& tokens);
    void preprocess_pre_end(const Token& directive, const std::vector<Token>& tokens);
    void preprocess_pre_else(const Token& directive, const std::vector<Token>& tokens);
    void preprocess_pre_if(const Token& directive, const std::vector<Token>& tokens);

    static int convert_digit(int c);
    static bool is_identifier_continuation(int c) {return is_identifier_start(c) || isdigit(c);}
    static bool is_identifier_start(int c) { return islower(c) || isupper(c) || c == '_'; }
    static bool is_identifier(const std::string& s);

    bool use_preprocessor;
    const Path& path;

    static const Token token_define;
    static const Token token_include;
    static const Token token_pre_else;
    static const Token token_pre_else_if;
    static const Token token_pre_end;
    static const Token token_pre_if;
    static const Token token_undefine;
    static const std::unordered_map<Token, PreprocessorDirective> preprocessor_directives;

    MatcherNode matcher;

    std::vector<Source> sources;
    Source* current_source{};
    Location eof_location;

    bool last_was_newline{true};
};

#endif // FILE_TOKENIZER_H
