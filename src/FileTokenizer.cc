/*
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
#include <ranges>

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/FileReader.h>
#include <tpau-cpp-kernal/Int.h>
#include <tpau-cpp-kernal/LocationException.h>
#include <tpau-cpp-kernal/UTF8.h>

#include "ExpressionParser.h"
#include "HexStringDecoder.h"
#include "Scope.h"
#include "SequenceTokenizer.h"
#include "Target.h"

using namespace tpau::cpp_kernal;

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

// TODO: is Visibility::ARGUMENT correct here?
FileTokenizer::FileTokenizer(const SearchPath& search_path, const Target* target, bool use_preprocessor, const std::unordered_set<Symbol>& defines) : preprocessor_scope(std::make_shared<Scope>(Visibility::ARGUMENT)), use_preprocessor{use_preprocessor}, search_path{search_path}, target{target} {
    preprocessor_scope->define(defines);
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
    sources.emplace_back(file_name);
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
            throw LocationException(token.location, "preprocessor directive in middle of line");
        }

        try {
            std::vector<Token> arguments = {};
            Token argument_token;
            while (((argument_token = next_raw())) && !argument_token.is_newline()) {
                arguments.emplace_back(argument_token);
            }
            preprocess(token, arguments);
        }
        catch (LocationException& ex) {
            DiagnosticOutput::global.error(ex);
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

        auto c = current_source->get();
        if (c == '{') {
            auto c2 = current_source->get();
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
        if (auto type = matcher.match(*current_source, name)) {
            current_source->expand_location(location);
            last_was_newline = false;
            return {*type, location, name};
        }

        current_source->reset_to(location);

        c = current_source->get();

        if (c == EOF) {
            eof_location = current_location();
            if (!current_source->pre_states.empty()) {
                DiagnosticOutput::global.error(eof_location, "unclosed .pre_if at end of file");
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
                location.end.column += 1;
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
            while (current_source->get() != '\n') {
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
            throw LocationException(location, "illegal character '{}'", UTF8::encode(c));
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
    return std::ranges::all_of(current_source->pre_states, [](auto state) { return state; });
}

Token FileTokenizer::parse_hex(Location location) {
    auto decoder = HexStringDecoder();

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->get();

        if (c == '}') {
            auto c2 = current_source->get();
            if (c2 != '}') {
                current_source->unget();
                throw LocationException(current_location(), "invalid character in hex data");
            }
            current_source->expand_location(location);
            try {
                return {location, Value(decoder.end(), true)};
            }
            catch (Exception& ex) {
                throw LocationException(current_location(), ex);
            }
        }

        try {
            decoder.decode(static_cast<char>(c));
        }
        catch (Exception& ex) {
            throw LocationException(current_location(), ex);
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
        auto c = current_source->get();
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
        if (digit < 0 || static_cast<unsigned int>(digit) >= base) {
            current_source->unget();
            if (in_fraction) {
                return {Token::VALUE, location, floating};
            }
            if (size == 0) {
                throw LocationException(location, "empty integer");
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
        auto c = current_source->get();

        if (is_identifier_start(c) || (!name.empty() && isdigit(c))) {
            name += static_cast<char>(c);
        }
        else {
            current_source->unget();
            if (name.empty()) {
                throw LocationException(location, "empty directive");
            }
            return {type, location, name};
        }
    }
}

Token FileTokenizer::parse_string(Location location) { return {Token::STRING, location, Symbol(parse_string_literal(location, '"'))}; }

Token FileTokenizer::parse_char(Location location) {
    auto value = parse_string_literal(location, '\'');

    auto encoding_name = Symbol{};
    auto c = current_source->get();
    if (c == ':') {
        encoding_name = parse_name(Token::NAME, location).as_symbol();
    }
    else {
        current_source->unget();
    }

    if (!target) {
        throw LocationException(location, "no string encoding");
    }
    auto encoding = target->default_string_encoding;
    if (encoding_name) {
        encoding = target->string_encoding(encoding_name);
    }
    if (!encoding) {
        if (encoding_name) {
            throw LocationException(location, "unknown string encoding {}", encoding_name);
        }
        else {
            throw LocationException(location, "no default string encoding");
        }
    }

    auto bytes = std::string{};
    encoding->encode(bytes, value);
    if (bytes.empty()) {
        throw LocationException(location, "empty character constant");
    }
    else if (bytes.size() > 1) {
        throw LocationException(location, "multi-byte character constant");
    }
    else {
        return {location, Value(static_cast<uint64_t>(static_cast<uint8_t>(bytes[0])))};
    }
}

std::string FileTokenizer::parse_string_literal(Location location, int terminator) {
    std::string value;

    while (true) {
        current_source->expand_location(location);
        auto c = current_source->get();

        if (c == terminator) {
            current_source->expand_location(location);
            return value;
        }

        switch (c) {
            case '\n':
                throw LocationException(location, "unterminated string");


            case '\\': {
                current_source->expand_location(location);
                switch (auto c2 = current_source->get()) {
                    case '\\':
                    case '\"':
                    case '\'':
                        value += static_cast<char>(c2);
                        break;

                    case 'n':
                        value += '\n';
                        break;

                    case '\n':
                        throw LocationException(location, "unterminated string");

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
        throw LocationException(directive.location, "unknown preprocessor directive");
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
            throw LocationException(filename_token.location, "file not found");
        }
        push(file);
    }
    catch (Exception& ex) {
        throw LocationException(filename_token.location, ex);
    }
}

void FileTokenizer::preprocess_pre_end(const Token& directive, const std::vector<Token>& arguments) {
    if (current_source->pre_states.empty()) {
        throw LocationException(directive.location, "{} outside .pre_if", directive);
    }
    current_source->pre_states.pop_back();
}

void FileTokenizer::preprocess_pre_else(const Token& directive, const std::vector<Token>& arguments) {
    if (current_source->pre_states.empty()) {
        throw LocationException(directive.location, "{} outside .pre_if", directive);
    }

    current_source->pre_states.back().process_else();
}

void FileTokenizer::preprocess_pre_if(const Token& directive, const std::vector<Token>& arguments) {
    if (directive == token_pre_else_if) {
        if (current_source->pre_states.empty()) {
            throw LocationException(directive.location, "{} outside .pre_if", directive);
        }
        if (current_source->pre_states.back().skipping_rest()) {
            current_source->pre_states.back().process_else_if(false);
            return;
        }
    }

    auto tokenizer = SequenceTokenizer{arguments};
    auto expression = ExpressionParser{tokenizer}.parse();

    expression.resolve(preprocessor_scope.get(), nullptr);
    // TODO: check if this is correct
    auto context = EvaluationContext{std::make_shared<Scope>(Visibility::ARGUMENT)};
    expression.evaluate(context);
    if (!expression.has_value()) {
        throw LocationException(directive.location, "condition in {} must be constant", directive);
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
    if (!std::ranges::all_of(std::views::drop(s, 1), is_identifier_continuation)) {
        return false;
    }
    if (!is_identifier_start(s.front()) && s.front() != '.') {
        return false;
    }
    return true;
}

Symbol FileTokenizer::find_file(Symbol file_name) { return search_path.find(file_name, current_source->location().file); }

std::optional<Token::Type> FileTokenizer::MatcherNode::match(FileSource& source, std::string& name) { // NOLINT(misc-no-recursion)
    auto c = source.get();
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
            c = source.get();
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

void FileTokenizer::MatcherNode::add(const char* string, Token::Type type, const std::unordered_set<char>& new_suffix, bool match_in_word_) { // NOLINT(misc-no-recursion)
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
    return std::ranges::any_of(new_suffix, [this](char c) { return next.contains(c); });
}

void FileTokenizer::PreprocessorDirective::operator()(FileTokenizer& tokenizer, const Token& directive, const std::vector<Token>& arguments) const {
    if (min_arguments && arguments.size() < min_arguments) {
        throw LocationException(directive.location, "too few arguments to '{}'", directive);
    }
    if (max_arguments && arguments.size() > max_arguments) {
        throw LocationException(directive.location, "too many arguments to '{}'", directive);
    }
    for (size_t i = 0; i < argument_type.size(); i++) {
        if (!argument_type[i].contains(arguments[i])) {
            throw LocationException(arguments[i].location, "expected {}", argument_type[i].name);
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

void FileTokenizer::define(Symbol name) { preprocessor_scope->define(name); }

void FileTokenizer::define(const std::unordered_set<Symbol>& defines) { preprocessor_scope->define(defines); }

void FileTokenizer::undefine(Symbol name) { preprocessor_scope->undefine(name); }

const std::unordered_set<Symbol>& FileTokenizer::get_defines() const { return preprocessor_scope->get_defines(); }
