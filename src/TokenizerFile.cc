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

#include "TokenizerFile.h"

#include "FileReader.h"
#include "ParseException.h"
#include "Util.h"

void TokenizerFile::push(const std::string& file_name) {
    const auto& lines = FileReader::global.read(file_name);
    if (lines.empty()) {
        return;
    }
    sources.emplace_back(SymbolTable::global[file_name], lines);
    current_source = &sources[sources.size() - 1];
}

Token TokenizerFile::sub_next() {
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
        else if (isword(c)) {
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

        if (isword(c) || (!name.empty()) && isdigit(c)) {
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
        source.unget();
        return match_type;
    }

    name += static_cast<char>(c);
    return it->second.match(source,name);
}


void TokenizerFile::MatcherNode::add(const char *string, Token::Type type) {
    if (string[0] == '\0') {
        if (match_type.has_value() && match_type.value() != type) {
            throw Exception("literal already defined with different type"); // TODO: include more detail
        }
        else {
            match_type = type;
        }
    }
    else {
        next[string[0]].add(string + 1, type);
    }
}
