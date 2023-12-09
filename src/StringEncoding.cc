/*
StringEncoding.cc --

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

#include "StringEncoding.h"
#include "Exception.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"
#include "Target.h"
#include "UTF8.h"

#if 0
#include <codecvt>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

// Use file-local variable instead of static class member to contain deprecation.
static auto encoder = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>();

#pragma clang diagnostic pop
#endif

const Token StringEncoding::token_base{Token::NAME, "base"};
const Token StringEncoding::token_name_delimiters{Token::NAME, "name_delimiters"};
const Token StringEncoding::token_named{Token::NAME, "named"};
const Token StringEncoding::token_named_ranges{Token::NAME, "named_ranges"};
const Token StringEncoding::token_ranges{Token::NAME, "ranges"};
const Token StringEncoding::token_singletons{Token::NAME, "singletons"};
const Token StringEncoding::token_use{Token::NAME, "use"};
const TokenGroup StringEncoding::group_char32{{Token::VALUE, Token::STRING}, {}, "unicode character"};


StringEncoding::StringEncoding(Symbol name, const std::shared_ptr<ParsedValue>& definition, const Target& target) {
    auto parameters = definition->as_dictionary();
    if (auto base = parameters->get_optional(token_base)) {
        if (auto base_dictionary = base->as_dictionary()) {
            for (const auto& pair: *base_dictionary) {
                import_base(pair.first, pair.second, target);
            }
        }
        else {
            throw ParseException(base->location, "base must be a dictionary");
        }
    }

    if (auto ranges = parameters->get_optional(token_ranges)) {
        if (ranges->is_scalar()) {
            add_range(ranges);
        }
        else if (ranges->is_array()) {
            for (const auto& range: *(ranges->as_array())) {
                add_range(range);
            }
        }
        else {
            throw ParseException(ranges->location, "ranges must be either scalar or array");
        }

        // TODO: sort and validate ranges
    }

    if (auto singletons = parameters->get_optional(token_singletons)) {
        if (auto singletons_dictionary = singletons->as_dictionary()) {
            for (const auto& pair: *singletons_dictionary) {
                add_singleton(pair.first, pair.second);
            }
        }
        else {
            throw ParseException(singletons->location, "singletons must be a dictionary");
        }
    }

    if (auto name_delimiters = parameters->get_optional(token_name_delimiters)) {
        set_name_delimiters(name_delimiters);
    }

    if (auto named_ranges_value = parameters->get_optional(token_named_ranges)) {
        if (auto named_ranges = named_ranges_value->as_dictionary()) {
            for (const auto& pair: *named_ranges) {
                add_named_ranges(pair.first, pair.second);
            }
        }
        else {
            throw ParseException(named_ranges_value->location, "named ranges must be dictionary");
        }
    }

    if (auto named = parameters->get_optional(token_named)) {
        if (auto named_dictionary = named->as_dictionary()) {
            for (const auto& pair: *named_dictionary) {
                add_named(pair.first, pair.second);
            }
        }
        else {
            throw ParseException(named->location, "named must be a dictionary");
        }
    }
}


#if 0
std::u32string StringEncoding::UTF8::decode(const std::string& string) {
    return encoder.from_bytes(string);
}

std::string StringEncoding::UTF8::encode(const std::u32string& string) {
    return encoder.to_bytes(string);
}

std::string StringEncoding::UTF8::encode(char32_t codepoint) {
    return UTF8::encode(std::u32string{codepoint});
}
#endif

size_t StringEncoding::encode(std::string* bytes, const std::string& string) const {
    auto codepoints = UTF8::decode(string);
    auto length = size_t{0};

    auto i = size_t{0};
    while (i < codepoints.size()) {
        if (has_named() && codepoints.compare(i, named_open.size(), named_open) == 0) {
            auto start = i + named_open.size();
            auto end = codepoints.find(named_close, start);
            if (end == std::u32string::npos) {
                throw Exception ("unclosed named character");
            }
            auto character_name = codepoints.substr(start, end - start);
            auto it = named.find(character_name);
            if (it == named.end()) {
                throw Exception("unknown named character '%s'", UTF8::encode(character_name).c_str());
            }
            length += 1;
            if (bytes) {
                *bytes += static_cast<char>(it->second);
            }
            i = end + named_close.size();
        }
        else {
            auto codepoint = codepoints[i];
            if (auto byte = encode(codepoint)) {
                length += 1;
                if (bytes) {
                    *bytes += static_cast<char>(*byte);
                }
            }
            else {
                throw Exception("unmapped character '%s'", UTF8::encode(codepoint).c_str());
            }
            i += 1;
        }
    }

    return length;
}

std::optional<uint8_t> StringEncoding::encode(char32_t codepoint) const {
    for (const auto& range: ranges) {
        if (auto byte = range.encode(codepoint)) {
            return *byte;
        }
    }

    auto it = singletons.find(codepoint);
    if (it != singletons.end()) {
        return it->second;
    }

    return {};
}

void StringEncoding::add_range(const std::shared_ptr<ParsedValue>& range) {
    if (auto parameters = range->as_scalar()) {
        auto length = std::optional<size_t>{};

        // target_start [- target_end] : source_start [- source_end]

        auto tokenizer = SequenceTokenizer(parameters->location, parameters->tokens);

        auto target_start = get_uint8(tokenizer);

        auto token = tokenizer.expect(Token::PUNCTUATION);
        if (token == Token::minus) {
            auto target_end = get_uint8(tokenizer);
            if (target_end < target_start) {
                throw ParseException(range->location, "end of target range before start");
            }
            length = target_end - target_start + 1;
            token = tokenizer.expect(Token::PUNCTUATION);
        }

        if (token != Token::colon) {
            throw ParseException(token, "'=' expected in range");
        }

        auto source_start = get_char32(tokenizer);

        token = tokenizer.next();
        if (token) {
            if (token != Token::minus) {
                throw ParseException(token, "expected '-'");
            }
            auto source_end = get_char32(tokenizer);
            if (source_end < source_start) {
                throw ParseException(range->location, "end of source range before start");
            }
            auto source_length = source_end - source_start + 1;
            if (length) {
                if (source_length != *length) {
                    throw ParseException(token, "source and target ranges differ in length");
                }
            }
            else {
                if (target_start + source_length > std::numeric_limits<uint8_t>::max() + 1) {
                    throw ParseException(token, "target range doesn't fit in one byte");
                }
                length = static_cast<uint8_t>(source_length);
            }
        }

        if (!length) {
            throw ParseException(parameters->location, "no end given for range");
        }

        ranges.emplace_back(source_start, target_start, *length);
    }
    else {
        throw ParseException(range->location, "range must be scalar");
    }
}

void StringEncoding::set_name_delimiters(const std::shared_ptr<ParsedValue>& delimiters) {
    if (auto parameters = delimiters->as_scalar()) {
        if (parameters->size() != 2) {
            throw ParseException(delimiters->location, "exactly two name_delimiters expected");
        }
        if (!(*parameters)[0].is_string() || !(*parameters)[1].is_string()) {
            throw ParseException(delimiters->location, "name_delimiters must be strings");
        }
        named_open = UTF8::decode((*parameters)[0].as_string());
        named_close = UTF8::decode((*parameters)[1].as_string());
    }
    else {
        throw ParseException(delimiters->location, "name_delimiters must be scalar");
    }
}

void StringEncoding::add_singleton(const Token& target_token, const std::shared_ptr<ParsedValue>& sources) {
    if (!target_token.is_unsigned() || target_token.as_unsigned() > std::numeric_limits<uint8_t>::max()) {
        throw ParseException(target_token, "invalid target for singleton");
    }
    auto target = get_uint8(target_token);
    if (auto sources_list = sources->as_scalar()) {
        for (const auto& source_token: *sources_list) {
            auto source = get_char32(source_token);
            try {
                add_singleton(source, target);
            }
            catch (Exception &ex) {
                throw ParseException(source_token, ex);
            }
        }
    }
    else {
        throw ParseException(target_token, "invalid sources for singleton");
    }
}

void StringEncoding::add_named(const Token& target_token, const std::shared_ptr<ParsedValue>& sources) {
    auto target = get_uint8(target_token);
    if (auto sources_list = sources->as_scalar()) {
        for (const auto& source_token: *sources_list) {
            if (!source_token.is_string()) {
                throw ParseException(source_token, "named source must be a string");
            }
            auto source = UTF8::decode(source_token.as_string());
            try {
                add_named(source, target);
            }
            catch (Exception& ex) {
                throw ParseException(source_token, ex);
            }
        }
    }
    else {
        throw ParseException(target_token, "invalid sources for named");
    }
}

void StringEncoding::import_base(const Token& base_token, const std::shared_ptr<ParsedValue>& base_parameters, const Target& target) {
    if (auto base_name = base_token.as_symbol()) {
        const StringEncoding* base = target.string_encoding(base_name);
        if (!base) {
            throw ParseException(base_token, "unknown encoding");
        }
        if (auto base_dict = base_parameters->as_dictionary()) {
            if (auto uses_v = base_dict->get_optional(token_use)) {
                if (auto uses = uses_v->as_array()) {
                    for (const auto& use_range_v: (*uses)) {
                        if (auto use_range = use_range_v->as_scalar()) {
                            if ((use_range->size() != 3 && use_range->size() != 5) || !(*use_range)[0].is_unsigned()|| (*use_range)[1] != Token::minus || !(*use_range)[2].is_unsigned()) {
                                throw ParseException(use_range_v->location, "invalid use range");
                            }
                            auto start = get_uint8((*use_range)[0]);
                            auto end = get_uint8((*use_range)[2]);
                            auto offset = uint64_t{};

                            if (use_range->size() == 5) {
                                if ((*use_range)[3] != Token::colon) {
                                    throw ParseException(use_range_v->location, "invalid use range");
                                }
                                offset = get_uint8((*use_range)[4]);
                            }

                            if (end - start + offset > std::numeric_limits<uint8_t>::max()) {
                                throw ParseException(use_range_v->location, "invalid use range");
                            }
                            try {
                                import_base_range(base, start, end, offset);
                            }
                            catch (Exception &ex) {
                                throw ParseException(use_range_v->location, ex);
                            }
                        }
                        else {
                            throw ParseException(use_range_v->location, "uses range must be scalar");
                        }
                    }
                }
                else {
                    throw ParseException(uses_v->location, "uses must be dictionary");
                }
            }

        }
        else {
            throw ParseException(base_token, "base must be dictionary");
        }
    }
    else {
        throw ParseException(base_token, "base name must be identifier");
    }
}

void StringEncoding::add_singleton(uint32_t source, uint8_t target) {
    if (singletons.contains(source)) {
        throw Exception("duplicate singleton source '%s'", UTF8::encode(source).c_str());
    }
    singletons[source] = target;
}

void StringEncoding::add_named(const std::u32string& source, uint8_t target) {
    if (named.contains(source)) {
        throw Exception("duplicate name '%s'", UTF8::encode(source).c_str());
    }
    named[source] = target;
}

void StringEncoding::import_base_range(const StringEncoding* base, uint8_t start, uint8_t end, uint8_t offset) {
    for (const auto& range: base->ranges) {
        if (start <= range.target_end() && end >= range.target_start) {
            uint8_t start_offset = std::max(start, range.target_start) - range.target_start;
            uint8_t end_offset = range.target_end() - std::min(end, range.target_end());
            add_range(CharacterRange(range.source_start + start_offset, range.target_start + start_offset + offset, range.length - start_offset - end_offset));
        }
    }
    for (const auto& pair: base->singletons) {
        if (is_in_range(pair.second, start, end)) {
            add_singleton(pair.first, pair.second + offset);
        }
    }
    for (const auto& pair: base->named) {
        if (is_in_range(pair.second, start, end)) {
            add_named(pair.first, pair.second + offset);
        }
    }
    named_open = base->named_open;
    named_close = base->named_close;
}

void StringEncoding::add_named_range(const std::u32string& prefix, char32_t source_start, uint8_t target_start, uint8_t length) {
    //std::cout << "adding named range prefix='" << UTF8::encode(prefix) << ", source_start='" << UTF8::encode(source_start) << "', target_start=" << (int)target_start << ", length=" << (int)length << "\n"; // DEUBG
    for (uint8_t offset = 0; offset < length; offset += 1) {
        //std::cout << "  adding '" << UTF8::encode(prefix + static_cast<char32_t>(source_start + offset)) << "': " << (int)(target_start + offset) << "\n"; // DEBUG
        add_named(prefix + static_cast<char32_t>(source_start + offset), target_start + offset);
    }
}

void StringEncoding::add_named_ranges(const Token& prefix_token, const std::shared_ptr<ParsedValue>& ranges_value) {
    if (!prefix_token.is_string()) {
        throw ParseException(prefix_token, "named ranges prefix must be string");
    }
    auto prefix = UTF8::decode(prefix_token.as_string());
    if (auto ranges_array = ranges_value->as_array()) {
        for (const auto& range: *ranges_array) {
            if (auto range_parameters = range->as_scalar()) {
                if (range_parameters->size() < 3 || !(*range_parameters)[0].is_unsigned() || (*range_parameters)[1] != Token::colon || !(*range_parameters)[2].is_string()) {
                    throw ParseException(range->location, "invalid named range");
                }
                auto target_start = get_uint8((*range_parameters)[0]);
                auto source_start = get_char32((*range_parameters)[2]);
                auto source_end = source_start;
                if (range_parameters->size() > 3) {
                    if (range_parameters->size() != 5 || (*range_parameters)[3] != Token::minus || !group_char32.contains((*range_parameters)[4])) {
                        throw ParseException(range->location, "invalid named range");
                    }
                    source_end = get_char32((*range_parameters)[4]);
                    if (source_end < source_start) {
                        throw ParseException(range->location, "invalid named range");
                    }
                }
                add_named_range(prefix, source_start, target_start, source_end - source_start + 1);
            }
            else {
                throw ParseException(range->location, "named range must be scalar");
            }
        }
    }
    else {
        throw ParseException(prefix_token, "named ranges must be array");
    }
}

std::optional<uint8_t> StringEncoding::CharacterRange::encode(char32_t codepoint) const {
    if (codepoint >= source_start) {
        auto offset = codepoint - source_start;
        if (offset < length) {
            return target_start + offset;
        }
    }

    return {};
}

char32_t StringEncoding::get_char32(Tokenizer& tokenizer) {
    return get_char32(tokenizer.expect(group_char32, TokenGroup::all));
}

char32_t StringEncoding::get_char32(const Token& token) {
    if (token.is_unsigned()) {
        auto value = token.as_unsigned();
        if (value > std::numeric_limits<char32_t>::max()) {
            throw ParseException(token, "unicode character out of range");
        }
        return static_cast<char32_t>(value);
    }
    else if (token.is_string()) {
        auto string = UTF8::decode(token.as_string());
        switch (string.size()) {
            case 0:
                throw ParseException(token, "unicode character expected, got empty string");

            case 1:
                return string[0];

            default:
                throw ParseException(token, "unicode character expected, got multi-character string");
        }
    }
    else {
        throw ParseException(token, "invalid unicode character");
    }
}

uint8_t StringEncoding::get_uint8(Tokenizer& tokenizer) {
    return get_uint8(tokenizer.expect(Token::VALUE));
}

uint8_t StringEncoding::get_uint8(const Token& token) {
    if (token.is_unsigned()) {
        auto value = token.as_unsigned();
        if (value > std::numeric_limits<uint8_t>::max()) {
            throw ParseException(token, "character out of range");
        }
        return static_cast<uint8_t>(value);
    }
    else {
        throw ParseException(token, "invalid character");
    }
}

size_t StringEncoding::encoded_size(const Value& value) const {
    return encoded_size(value.string_value());
}

void StringEncoding::encode(std::string& bytes, const Value& value) const {
    return encode(bytes, value.string_value());
}

std::ostream& operator<<(std::ostream& stream, const StringEncoding& encoding) {
    encoding.serialize(stream);
    return stream;
}

void StringEncoding::serialize(std::ostream& stream) const {
    stream << name;
}