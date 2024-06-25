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

#include "FileTokenizer.h"

#include <algorithm>

#include "ExpressionParser.h"
#include "FileReader.h"
#include "HexStringDecoder.h"
#include "Int.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"
#include "Target.h"

const Token FileTokenizer::token_define{Token::Type::PREPROCESSOR, ".define"};
const Token FileTokenizer::token_include{Token::Type::PREPROCESSOR, ".include"};
const Token FileTokenizer::token_pre_end{Token::Type::PREPROCESSOR, ".pre_end"};
const Token FileTokenizer::token_pre_else{Token::Type::PREPROCESSOR, ".pre_else"};
const Token FileTokenizer::token_pre_else_if{Token::Type::PREPROCESSOR, ".pre_else_if"};
const Token FileTokenizer::token_pre_if{Token::Type::PREPROCESSOR, ".pre_if"};
const Token FileTokenizer::token_undefine{Token::Type::PREPROCESSOR, ".undefine"};
// clang-format off
const std::unordered_map<Token, FileTokenizer::PreprocessorDirective> FileTokenizer::preprocessor_directives = {
    {token_define, PreprocessorDirective{1, 1, {TokenGroup{Token::NAME}}, &FileTokenizer::preprocess_define}},
    {token_include, PreprocessorDirective{1, 1, {TokenGroup{Token::STRING}}, &FileTokenizer::preprocess_include}},
    {token_pre_end, PreprocessorDirective{0, 0, {}, &FileTokenizer::preprocess_pre_end}},
    {token_pre_else, PreprocessorDirective{0, 0, {}, &FileTokenizer::preprocess_pre_else}},
    {token_pre_else_if, PreprocessorDirective{1, {}, {}, &FileTokenizer::preprocess_pre_if}},
    {token_pre_if, PreprocessorDirective{1, {}, {}, &FileTokenizer::preprocess_pre_if}},
    {token_undefine, PreprocessorDirective{1, 1, {TokenGroup{Token::NAME}}, &FileTokenizer::preprocess_define}}
};
// clang-format on

FileTokenizer::FileTokenizer(const Path& path, const Target* target, bool use_preprocessor, const std::unordered_set<Symbol>& defines) : path{path}, use_preprocessor{use_preprocessor}, defines{defines}, target{target} {
    if (use_preprocessor) {
        add_literal(token_define);
        add_literal(token_include);
        add_literal(token_pre_else);
        add_literal(token_pre_else_if);
        add_literal(token_pre_end);
        add_literal(token_pre_if);
        add_literal(token_undefine);
    }
}

void FileTokenizer::push(Symbol file_name) {
    const auto& lines = FileReader::global.read(file_name);
    if (lines.empty()) {
        return;
    }
    sources.emplace_back(file_name, lines);
    current_source = &sources[sources.size() - 1];
}

Token FileTokenizer::sub_next() {
    while (true) {
        bool beginning_of_line = last_was_newline;

        auto token = next_raw();

        if (!use_preprocessor || !token.is_preprocessor()) {
            if (pre_is_processing()) {
                return token;
            }
            else {
                continue;
            }
        }

        if (!beginning_of_line) {
            throw ParseException(token, "preprocessor directive in middle of line");
        }

        try {
            std::vector<Token> arguments = {};
            Token argument_token;
            while (((argument_token = next_raw())) && !argument_token.is_newline()) {
                arguments.emplace_back(argument_token);
            }
            preprocess(token, arguments);
        } catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
        }
    }
}

void FileTokenizer::PreState::process_else() {
    if (else_seen) {
        throw Exception(".pre_else after .pre_else");
    }
    if (skip_rest) {
        processing = false;
    }
    else {
        processing = !processing;
    }
    else_seen = true;
}

void FileTokenizer::PreState::process_else_if(bool condition) {
    if (else_seen) {
        throw Exception(".pre_else_if after .pre_else");
    }
    if (skip_rest) {
        processing = false;
    }
    else {
        processing = condition;
        skip_rest = condition;
    }
}

Token FileTokenizer::next_raw() {
    if (current_source == nullptr) {
        return {};
    }

    while (true) {
        auto location = current_source->location();

        auto c = current_source->next();
        if (c == '{') {
            auto c2 = current_source->next();
            if (c2 == '{') {
                return parse_hex(location);
            }
            else {
                current_source->unget();
                current_source->unget();
            }
        }
        else {
            current_source->unget();
        }

        std::string name;
        auto type = matcher.match(*current_source, name);
        if (type) {
            current_source->expand_location(location);
            last_was_newline = false;
            return {*type, location, name};
        }

        current_source->reset_to(location);

        c = current_source->next();

        if (c == EOF) {
            eof_location = current_location();
            if (!current_source->pre_states.empty()) {
                FileReader::global.error(eof_location, "unclosed .pre_if at end of file");
            }
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
            while (current_source->next() != '\n') {
            }
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
        else if (c == '\'') {
            return parse_char(location);
        }
        else {
            throw ParseException(location, "illegal character '%c'", c);
        }
    }
}

bool FileTokenizer::pre_is_processing() const {
    if (!current_source) {
        return true;
    }
    if (current_source->pre_states.empty()) {
        return true;
    }
    return std::all_of(current_source->pre_states.begin(), current_source->pre_states.end(), [](auto state){return state;});
}

Token FileTokenizer::parse_hex(Location location) {
    auto decoder = HexStringDecoder();

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();

        if (c == '}') {
            auto c2 = current_source->next();
            if (c2 != '}') {
                current_source->unget();
                throw ParseException(current_location(), "invalid character in hex data");
            }
            current_source->expand_location(location);
            try {
                return {location, Value(decoder.end())};
            }
            catch (Exception& ex) {
                throw ParseException(current_location(), ex);
            }
        }

        try {
            decoder.decode(static_cast<char>(c));
        }
        catch (Exception& ex) {
            throw ParseException(current_location(), ex);
        }
    }
}

Token FileTokenizer::parse_number(unsigned int base, Location location) {
    uint64_t size = 0;
    auto leading_zero = false;
    uint64_t integer = 0;
    double floating = 0;
    auto in_fraction = false;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();
        if (c == '.') {
            if (in_fraction) {
                current_source->unget();
                return {Token::VALUE, location, floating};
            }
            else {
                in_fraction = true;
                floating = static_cast<double>(integer);
                size = 0;
            }
            continue;
        }
        // TODO: exponent
        int digit = convert_digit(c);
        if (digit < 0 || digit >= base) {
            current_source->unget();
            if (in_fraction) {
                return {Token::VALUE, location, floating};
            }
            if (size == 0) {
                throw ParseException(location, "empty integer");
            }
            uint64_t byte_size = 0;
            if (size > 1 && leading_zero) {
                byte_size = Int::minimum_byte_size(static_cast<uint64_t>((1 << size * (base == 16 ? 4 : 1)) - 1));
            }
            return {Token::VALUE, location, integer, byte_size};
        }

        if (size == 0 && digit == 0 && (base == 2 || base == 16)) {
            leading_zero = true;
        }
        if (in_fraction) {
            size += 10;
            floating += static_cast<double>(digit) / static_cast<double>(size);
        }
        else {
            size += 1;
            integer = integer * base + digit;
        }
    }
}

int FileTokenizer::convert_digit(int c) {
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

Token FileTokenizer::parse_name(Token::Type type, Location location) {
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
            return {type, location, name};
        }
    }
}

Token FileTokenizer::parse_string(Location location) {
    return {Token::STRING, location, Symbol(parse_string_literal(location, '"'))};
}

Token FileTokenizer::parse_char(Location location) {
    auto value= parse_string_literal(location, '\'');

    auto encoding_name = Symbol{};
    auto c = current_source->next();
    if (c == ':') {
        encoding_name = parse_name(Token::NAME, location).as_symbol();
    }
    else {
        current_source->unget();
    }

    if (!target) {
        throw ParseException(location, "no string encoding");
    }
    auto encoding = target->default_string_encoding;
    if (encoding_name) {
        encoding = target->string_encoding(encoding_name);
    }
    if (!encoding) {
        if (encoding_name) {
            throw ParseException(location, "unknown string encoding %s", encoding_name.c_str());
        }
        else {
            throw ParseException(location, "no default string encoding");
        }
    }

    auto bytes = std::string{};
    encoding->encode(bytes, value);
    if (bytes.empty()) {
        throw ParseException(location, "empty character constant");
    }
    else if (bytes.size() > 1) {
        throw ParseException(location, "multi-byte character constant");
    }
    else {
        return Token(location, Value(static_cast<uint64_t>(static_cast<uint8_t>(bytes[0]))));
    }
}

std::string FileTokenizer::parse_string_literal(Location location, int terminator){
    std::string value;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->next();

        if (c == terminator) {
            current_source->expand_location(location);
            return value;
        }

        switch (c) {
            case '\n':
                throw ParseException(location, "unterminated string");


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

Location FileTokenizer::current_location() const {
    if (current_source == nullptr) {
        return eof_location;
    }
    else {
        return current_source->location();
    }
}

void FileTokenizer::add_punctuations(const std::unordered_set<std::string>& names) {
    for (const auto& name : names) {
        add_literal(Token::PUNCTUATION, name);
    }
}

void FileTokenizer::preprocess(const Token& directive, const std::vector<Token>& arguments) {
    auto it = preprocessor_directives.find(directive);

    if (it == preprocessor_directives.end()) {
        throw ParseException(directive, "unknown preprocessor directive");
    }

    it->second(*this, directive, arguments);
}

void FileTokenizer::preprocess_define(const Token& directive, const std::vector<Token>& arguments) {
    if (!pre_is_processing()) {
        return;
    }
    const auto& name = arguments[0];
    if (directive == token_define) {
        define(name.as_symbol());
    }
    else {
        undefine(name.as_symbol());
    }
}

void FileTokenizer::preprocess_include(const Token& directive, const std::vector<Token>& arguments) {
    const auto& filename_token = arguments[0];

    try {
        auto file = find_file(filename_token.as_symbol());
        if (file.empty()) {
            throw ParseException(filename_token, "file not found");
        }
        push(file);
    } catch (Exception& ex) {
        throw ParseException(filename_token, "%s", ex.what());
    }
}

void FileTokenizer::preprocess_pre_end(const Token& directive, const std::vector<Token>& arguments) {
    if (current_source->pre_states.empty()) {
        throw ParseException(directive, "%s outside .pre_if", directive.as_string().c_str());
    }
    current_source->pre_states.pop_back();
}

void FileTokenizer::preprocess_pre_else(const Token& directive, const std::vector<Token>& arguments) {
    if (current_source->pre_states.empty()) {
        throw ParseException(directive, "%s outside .pre_if", directive.as_string().c_str());
    }

    current_source->pre_states.back().process_else();
}

void FileTokenizer::preprocess_pre_if(const Token& directive, const std::vector<Token>& arguments) {
    if (directive == token_pre_else_if) {
        if (current_source->pre_states.empty()) {
            throw ParseException(directive, "%s outside .pre_if", directive.as_string().c_str());
        }
        if (current_source->pre_states.back().skipping_rest()) {
            current_source->pre_states.back().process_else_if(false);
            return;
        }
    }

    auto tokenizer = SequenceTokenizer{arguments};
    auto expression = ExpressionParser{tokenizer}.parse();
    auto result = EvaluationResult{};
    auto context = EvaluationContext{result, EvaluationContext::STANDALONE, std::make_shared<Environment>(), defines};
    expression.evaluate(context);
    if (!expression.has_value()) {
        throw ParseException(directive, "condition in %s must be constant", directive.as_string().c_str());
    }
    auto condition = expression.value()->boolean_value();
    if (directive == token_pre_else_if) {
        current_source->pre_states.back().process_else_if(condition);
    }
    else {
        current_source->pre_states.emplace_back(condition);
    }
}

bool FileTokenizer::is_identifier(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    if (!std::all_of(s.begin(), s.end(), is_identifier_continuation)) {
        return false;
    }
    if (!is_identifier_start(s.front())) {
        return false;
    }
    return true;
}

Symbol FileTokenizer::find_file(Symbol file_name) { return path.find(file_name, current_source->location().file); }

void FileTokenizer::define(const std::unordered_set<Symbol>& defines) {
    for (const auto& name : defines) {
        define(name);
    }
}

int FileTokenizer::Source::next() {
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

void FileTokenizer::Source::unget() {
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

void FileTokenizer::Source::reset_to(const Location& new_location) {
    if (new_location.file != file_) {
        throw ParseException(location(), "can't reset to new_location in different file");
    }
    line = new_location.start_line_number - 1;
    column = new_location.start_column;
}

std::optional<Token::Type> FileTokenizer::MatcherNode::match(FileTokenizer::Source& source, std::string& name) {
    auto c = source.next();
    if (c == EOF) {
        if (name.empty()) {
            // don't match empty string
            return {};
        }
        return match_type;
    }

    auto it = next.find(static_cast<char>(c));

    if (it == next.end()) {
        while (suffix_characters.contains(static_cast<char>(c))) {
            name += static_cast<char>(c);
            c = source.next();
            if (c == EOF) {
                break;
            }
        }

        bool matched;

        if (name.empty() || !match_type.has_value()) {
            // don't match empty string
            matched = false;
        }
        else if (!match_in_word && is_identifier_continuation(c)) {
            // don't match prefix of longer identifier (if matched string is valid identifier)
            matched = !is_identifier(name);
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
    return it->second->match(source, name);
}

void FileTokenizer::MatcherNode::add(const char* string, Token::Type type, const std::unordered_set<char>& new_suffix, bool match_in_word_) {
    if (string[0] == '\0') {
        if (match_type.has_value() && (match_type.value() != type || match_in_word != match_in_word_)) {
            throw Exception("literal already defined with different type"); // TODO: include more detail
        }
        if (conflicts(new_suffix)) {
            throw Exception("suffix_characters conflicts with already defined literal"); // TODO: include more detail
        }
        if (match_type && suffix_characters != new_suffix) {
            throw Exception("literal already defined with different suffix characters"); // TODO: include more detail
        }

        match_in_word = match_in_word_;
        match_type = type;
        suffix_characters = new_suffix;
    }
    else {
        if (suffix_characters.contains(string[0])) {
            throw Exception("literal conflicts with already defined suffix characters"); // TODO include more detail
        }
        if (!next.contains(string[0])) {
            next[string[0]] = std::make_unique<MatcherNode>();
        }
        next[string[0]]->add(string + 1, type, new_suffix, match_in_word_);
    }
}

bool FileTokenizer::MatcherNode::conflicts(const std::unordered_set<char>& new_suffix) const {
    return std::any_of(new_suffix.begin(), new_suffix.end(), [this](char c) { return next.contains(c); });
}

void FileTokenizer::PreprocessorDirective::operator()(FileTokenizer& tokenizer, const Token& directive, const std::vector<Token>& arguments) const {
    if (min_arguments && arguments.size() < min_arguments) {
        throw ParseException(directive, "too few arguments to '%s'", directive.as_string().c_str());
    }
    if (max_arguments && arguments.size() > max_arguments) {
        throw ParseException(directive, "too many arguments to '%s'", directive.as_string().c_str());
    }
    for (size_t i = 0; i < argument_type.size(); i++) {
        if (!argument_type[i].contains(arguments[i])) {
            throw ParseException(arguments[i], "expected %s", argument_type[i].name.c_str());
        }
    }

    (tokenizer.*handler)(directive, arguments);
}

void FileTokenizer::add_literal(Token::Type match, const std::string& name, const std::string& suffix_characters) {
    auto suffix_set = std::unordered_set<char>();
    for (auto c : suffix_characters) {
        suffix_set.insert(c);
    }
    matcher.add(name.c_str(), match, suffix_set);
}
