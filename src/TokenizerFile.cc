/*
Tokenizer.cc -- Convert File into Stream of Tokens

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

#include "TokenizerFile.h"

#include "FileReader.h"
#include "ParseException.h"
#include "Util.h"

bool TokenizerFile::initialized = false;
Token TokenizerFile::token_include;

void TokenizerFile::push(const std::string& file_name) {
    const auto& lines = FileReader::global.read(file_name);
    if (lines.empty()) {
        return;
    }
    sources.emplace_back(SymbolTable::global[file_name], lines);
    current_source = &sources[sources.size() - 1];
}

Token TokenizerFile::sub_next() {
    while (true) {
        bool beginning_of_line = last_was_newline;

        auto token = next_raw();

        if (!use_preprocessor || !token.is_preprocessor()) {
            return token;
        }

        if (!beginning_of_line) {
            throw ParseException(token, "preprocessor directive in middle of line");
        }

        try {
            std::vector<Token> tokens = {token};
            while ((token = next_raw()) && !token.is_newline()) {
                tokens.emplace_back(token);
            }
            preprocess(tokens);
        }
        catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
        }
    }
}

Token TokenizerFile::next_raw() {
    if (current_source == nullptr) {
        return {};
    }

    while (true) {
        auto location = current_source->location();

        std::string name;
        auto type = matcher.match(*current_source, name);
        if (type.has_value()) {
            current_source->expand_location(location);
            last_was_newline = false;
            return {type.value(), location, SymbolTable::global.add(name)};
        }

        current_source->reset_to(location);

        auto c = current_source->next();

        if (c == EOF) {
            current_source = nullptr;
            sources.pop_back();
            if (sources.empty()) {
                return {};
            }
            current_source = &sources[sources.size() - 1];
            continue;
        }

        if (c == '\n') {
            if (last_was_newline) {
                // ignore empty lines
                continue;
            }
            else {
                // Not using expand_location() here to correctly handle \n.
                location.end_column += 1;
                last_was_newline = true;
                return {Token::NEWLINE, location};
            }
        }
        else if (isspace(c)) {
            // skip whitespace
            continue;
        }
        else if (c == ';') {
            // skip comments
            while (current_source->next() != '\n') {}
            current_source->unget();
            continue;
        }

        last_was_newline = false;

        if (c == '$') {
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
        else if (is_identifier_start(c)) {
            current_source->unget();
            return parse_name(Token::NAME, location);
        }
        else if (c == '"') {
            return parse_string(location);
        }
        else {
            throw ParseException(location, "illegal character '%c'", c);
        }
    }
}

Token TokenizerFile::parse_number(unsigned int base, Location location) {
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
                throw ParseException(location, "empty integer");
            }
            return {Token::INTEGER, location, integer};
        }

        empty = false;
        integer = integer * base + digit;
    }
}

int TokenizerFile::convert_digit(int c) {
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

Token TokenizerFile::parse_name(Token::Type type, Location location) {
    std::string name;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();

        if (is_identifier_start(c) || (!name.empty()) && isdigit(c)) {
            name += static_cast<char>(c);
        }
        else {
            current_source->unget();
            if (name.empty()) {
                throw ParseException(location, "empty directive");
            }
            return {type, location, SymbolTable::global.add(name)};
        }
    }

}

Token TokenizerFile::parse_string(Location location) {
    std::string value;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();

        switch (c) {
            case '\n':
                throw ParseException(location, "unterminated string");

            case '"':
                current_source->expand_location(location);
                return {Token::STRING, location, SymbolTable::global.add(value)};

            case '\\': {
                current_source->expand_location(location);
                auto c2 = current_source->next();
                switch (c2) {
                    case '\\':
                    case '\"':
                    case '\'':
                        value += static_cast<char>(c2);
                        break;

                    case 'n':
                        value += '\n';
                        break;

                    case '\n':
                        throw ParseException(location, "unterminated string");

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

Location TokenizerFile::current_location() const{
    if (current_source == nullptr) {
        return {};
    }
    else {
        return current_source->location();
    }
}

void TokenizerFile::add_punctuations(const std::unordered_set<std::string> &names) {
    for (const auto& name: names) {
        add_literal(Token::PUNCTUATION, name);
    }
}

TokenizerFile::TokenizerFile(std::shared_ptr<const Path> path, bool use_preprocessor): path(path), use_preprocessor(use_preprocessor) {
    if (use_preprocessor) {
        if (!initialized) {
            token_include = Token(Token::PREPROCESSOR, {}, SymbolTable::global.add(".include"));
            initialized = true;
        }
        matcher.add(".include", Token::PREPROCESSOR);
    }
}

void TokenizerFile::preprocess(const std::vector<Token>& tokens) {
    const auto& directive = tokens.front();

    if (directive == token_include) {
        if (tokens.size() < 2) {
            throw ParseException(directive, "missing argument for %s", directive.as_string().c_str());
        }
        if (tokens.size() > 2) {
            throw ParseException(directive, "too many arguments for %s", directive.as_string().c_str());
        }
        const auto& filename_token = tokens[1];

        if (!filename_token.is_string()) {
            throw ParseException(filename_token, "expected string");
        }

        try {
            auto filename = filename_token.as_string();
            auto file = path->find(filename, SymbolTable::global[current_source->location().file]);
            if (!file.has_value()) {
                throw ParseException(filename_token, "file not found");
            }
            push(file.value());
        }
        catch (Exception& ex) {
            throw ParseException(filename_token, "%s", ex.what());
        }
    }
    else {
        throw ParseException(directive, "unknown preprocessor directive");
    }
}


int TokenizerFile::Source::next() {
    if (line >= lines.size()) {
        return EOF;
    }

    if (column >= lines[line].size()) {
        line += 1;
        column = 0;
        return '\n';
    }

    auto c = lines[line][column];
    column += 1;
    return c;
}

void TokenizerFile::Source::unget() {
    if (column == 0) {
        if (line > 0) {
            line -= 1;
            column = lines[line].size();
        }
    }
    else {
        column -= 1;
    }
}

void TokenizerFile::Source::reset_to(const Location &new_location) {
    if (new_location.file != file) {
        throw ParseException(location(), "can't reset to new_location in different file");
    }
    line = new_location.start_line_number - 1;
    column = new_location.start_column;
}


std::optional<Token::Type> TokenizerFile::MatcherNode::match(TokenizerFile::Source &source, std::string& name) {
    auto c = source.next();
    if (c == EOF) {
        return match_type;
    }

    auto it = next.find(static_cast<char>(c));

    if (it == next.end()) {
        bool matched;

        if (name.empty()) {
            // don't match empty string
            matched = false;
        }
        else if (!match_in_word && is_identifier_continuation(c)) {
            // don't match prefix of longer identifier (if matched string is valid identifier)
            matched = !(is_identifier_start((name[0]) && std::all_of(name.begin() + 1, name.end(),
                                                                   is_identifier_continuation)));
        }
        else {
            // match if next character can't be part of identifier, or we're matching inside longer identifiers.
            matched = true;
        }

        if (matched) {
            source.unget();
            return match_type;
        }
        else {
            return {};
        }
    }

    name += static_cast<char>(c);
    return it->second.match(source,name);
}


void TokenizerFile::MatcherNode::add(const char *string, Token::Type type, bool match_in_word_) {
    if (string[0] == '\0') {
        if (match_type.has_value() && (match_type.value() != type || match_in_word != match_in_word_)) {
            throw Exception("literal already defined with different type"); // TODO: include more detail
        }
        else {
            match_in_word = match_in_word_;
            match_type = type;
        }
    }
    else {
        next[string[0]].add(string + 1, type);
    }
}
