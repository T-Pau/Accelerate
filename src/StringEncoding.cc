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


#include <tpau-cpp-kernal/LocationException.h>
#include <tpau-cpp-kernal/UTF8.h>

#include "StringEncoding.h"
#include "SequenceTokenizer.h"
#include "StructuredArray.h"
#include "StructuredDictionary.h"
#include "StructuredScalar.h"
#include "Target.h"

using namespace tpau::cpp_kernal;

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


StringEncoding::StringEncoding(Symbol name, const std::shared_ptr<StructuredValue>& definition, const Target& target) {
    auto parameters = definition->as_dictionary();
    if (auto base = parameters->get_optional(token_base)) {
        if (auto base_dictionary = base->as_dictionary()) {
            for (const auto& pair: *base_dictionary) {
                import_base(pair.first, pair.second, target);
            }
        }
        else {
            throw LocationException(base->location, "base must be a dictionary");
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
            throw LocationException(ranges->location, "ranges must be either scalar or array");
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
            throw LocationException(singletons->location, "singletons must be a dictionary");
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
            throw LocationException(named_ranges_value->location, "named ranges must be dictionary");
        }
    }

    if (auto named = parameters->get_optional(token_named)) {
        if (auto named_dictionary = named->as_dictionary()) {
            for (const auto& pair: *named_dictionary) {
                add_named(pair.first, pair.second);
            }
        }
        else {
            throw LocationException(named->location, "named must be a dictionary");
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
                throw Exception("unknown named character '{}'", UTF8::encode(character_name));
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
                throw Exception("unmapped character '{}'", UTF8::encode(codepoint, true));
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

void StringEncoding::add_range(const std::shared_ptr<StructuredValue>& range) {
    if (auto parameters = range->as_scalar()) {
        auto length = std::optional<uint8_t>{};

        // target_start [- target_end] : source_start [- source_end]

        auto tokenizer = SequenceTokenizer(parameters->location, parameters->tokens);

        auto target_start = get_uint8(tokenizer);

        auto token = tokenizer.expect(Token::PUNCTUATION);
        if (token == Token::minus) {
            auto target_end = get_uint8(tokenizer);
            if (target_end < target_start) {
                throw LocationException(range->location, "end of target range before start");
            }
            length = target_end - target_start + 1;
            token = tokenizer.expect(Token::PUNCTUATION);
        }

        if (token != Token::colon) {
            throw LocationException(token.location, "'=' expected in range");
        }

        auto source_start = get_char32(tokenizer);

        token = tokenizer.next();
        if (token) {
            if (token != Token::minus) {
                throw LocationException(token.location, "expected '-'");
            }
            auto source_end = get_char32(tokenizer);
            if (source_end < source_start) {
                throw LocationException(range->location, "end of source range before start");
            }
            size_t source_length = source_end - source_start + 1;
            if (length) {
                if (source_length != *length) {
                    throw LocationException(token.location, "source and target ranges differ in length");
                }
            }
            else {
                if (source_length > std::numeric_limits<uint8_t>::max() || target_start + source_length > std::numeric_limits<uint8_t>::max() + 1) {
                    throw LocationException(token.location, "target range doesn't fit in one byte");
                }
                length = static_cast<uint8_t>(source_length);
            }
        }

        if (!length) {
            throw LocationException(parameters->location, "no end given for range");
        }

        ranges.emplace_back(source_start, target_start, *length);
    }
    else {
        throw LocationException(range->location, "range must be scalar");
    }
}

void StringEncoding::set_name_delimiters(const std::shared_ptr<StructuredValue>& delimiters) {
    if (auto parameters = delimiters->as_scalar()) {
        if (parameters->size() != 2) {
            throw LocationException(delimiters->location, "exactly two name_delimiters expected");
        }
        if (!(*parameters)[0].is_string() || !(*parameters)[1].is_string()) {
            throw LocationException(delimiters->location, "name_delimiters must be strings");
        }
        named_open = UTF8::decode((*parameters)[0].as_string());
        named_close = UTF8::decode((*parameters)[1].as_string());
    }
    else {
        throw LocationException(delimiters->location, "name_delimiters must be scalar");
    }
}

void StringEncoding::add_singleton(const Token& target_token, const std::shared_ptr<StructuredValue>& sources) {
    if (!target_token.is_unsigned() || target_token.as_unsigned() > std::numeric_limits<uint8_t>::max()) {
        throw LocationException(target_token.location, "invalid target for singleton");
    }
    auto target = get_uint8(target_token);
    if (auto sources_list = sources->as_scalar()) {
        for (const auto& source_token: *sources_list) {
            auto source = get_char32(source_token);
            try {
                add_singleton(source, target);
            }
            catch (Exception &ex) {
                throw LocationException(source_token.location, ex);
            }
        }
    }
    else {
        throw LocationException(target_token.location, "invalid sources for singleton");
    }
}

void StringEncoding::add_named(const Token& target_token, const std::shared_ptr<StructuredValue>& sources) {
    auto target = get_uint8(target_token);
    if (auto sources_list = sources->as_scalar()) {
        for (const auto& source_token: *sources_list) {
            if (!source_token.is_string()) {
                throw LocationException(source_token.location, "named source must be a string");
            }
            auto source = UTF8::decode(source_token.as_string());
            try {
                add_named(source, target);
            }
            catch (Exception& ex) {
                throw LocationException(source_token.location, ex);
            }
        }
    }
    else {
        throw LocationException(target_token.location, "invalid sources for named");
    }
}

void StringEncoding::import_base(const Token& base_token, const std::shared_ptr<StructuredValue>& base_parameters, const Target& target) {
    if (auto base_name = base_token.as_symbol()) {
        const StringEncoding* base = target.string_encoding(base_name);
        if (!base) {
            throw LocationException(base_token.location, "unknown encoding");
        }
        if (auto base_dict = base_parameters->as_dictionary()) {
            if (auto uses_v = base_dict->get_optional(token_use)) {
                if (auto uses = uses_v->as_array()) {
                    for (const auto& use_range_v: (*uses)) {
                        if (auto use_range = use_range_v->as_scalar()) {
                            if (use_range->empty() || !(*use_range)[0].is_unsigned()) {
                                throw LocationException(use_range_v->location, "invalid use range");
                            }

                            auto start = get_uint8((*use_range)[0]);
                            auto end = start;
                            auto offset = uint8_t{};
                            switch (use_range->size()) {
                                case 1:
                                    break;

                                case 3:
                                    if ((*use_range)[1] == Token::minus) {
                                        end = get_uint8((*use_range)[2]);
                                    }
                                    else if ((*use_range)[1] == Token::colon) {
                                        offset = get_uint8((*use_range)[2]);
                                    }
                                    else {
                                        throw LocationException(use_range_v->location, "invalid use range");
                                    }
                                    break;

                                case 5:
                                    if ((*use_range)[1] == Token::minus && (*use_range)[3] == Token::colon) {
                                        end = get_uint8((*use_range)[2]);
                                        offset = get_uint8((*use_range)[4]) - start;
                                    }
                                    else {
                                        throw LocationException(use_range_v->location, "invalid use range");
                                    }
                                    break;

                                default:
                                    throw LocationException(use_range_v->location, "invalid use range");
                            }

                            if (end - start + offset > std::numeric_limits<uint8_t>::max()) {
                                throw LocationException(use_range_v->location, "invalid use range");
                            }
                            try {
                                import_base_range(base, start, end, offset);
                            }
                            catch (Exception &ex) {
                                throw LocationException(use_range_v->location, ex);
                            }
                        }
                        else {
                            throw LocationException(use_range_v->location, "uses range must be scalar");
                        }
                    }
                }
                else {
                    throw LocationException(uses_v->location, "uses must be dictionary");
                }
            }

        }
        else {
            throw LocationException(base_token.location, "base must be dictionary");
        }
    }
    else {
        throw LocationException(base_token.location, "base name must be identifier");
    }
}

void StringEncoding::add_singleton(uint32_t source, uint8_t target) {
    if (singletons.contains(source)) {
        throw Exception("duplicate singleton source '{}'", UTF8::encode(source));
    }
    singletons[source] = target;
}

void StringEncoding::add_named(const std::u32string& source, uint8_t target) {
    if (named.contains(source)) {
        throw Exception("duplicate name '{}'", UTF8::encode(source));
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
    for (uint8_t offset = 0; offset < length; offset += 1) {
        add_named(prefix + static_cast<char32_t>(source_start + offset), target_start + offset);
    }
}

void StringEncoding::add_named_ranges(const Token& prefix_token, const std::shared_ptr<StructuredValue>& ranges_value) {
    if (!prefix_token.is_string()) {
        throw LocationException(prefix_token.location, "named ranges prefix must be string");
    }
    auto prefix = UTF8::decode(prefix_token.as_string());
    if (auto ranges_array = ranges_value->as_array()) {
        for (const auto& range: *ranges_array) {
            if (auto range_parameters = range->as_scalar()) {
                if (range_parameters->size() < 3 || !(*range_parameters)[0].is_unsigned() || (*range_parameters)[1] != Token::colon || !(*range_parameters)[2].is_string()) {
                    throw LocationException(range->location, "invalid named range");
                }
                auto target_start = get_uint8((*range_parameters)[0]);
                auto source_start = get_char32((*range_parameters)[2]);
                auto source_end = source_start;
                if (range_parameters->size() > 3) {
                    if (range_parameters->size() != 5 || (*range_parameters)[3] != Token::minus || !group_char32.contains((*range_parameters)[4])) {
                        throw LocationException(range->location, "invalid named range");
                    }
                    source_end = get_char32((*range_parameters)[4]);
                    if (source_end < source_start) {
                        throw LocationException(range->location, "invalid named range");
                    }
                }
                add_named_range(prefix, source_start, target_start, source_end - source_start + 1);
            }
            else {
                throw LocationException(range->location, "named range must be scalar");
            }
        }
    }
    else {
        throw LocationException(prefix_token.location, "named ranges must be array");
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
            throw LocationException(token.location, "unicode character out of range");
        }
        return static_cast<char32_t>(value);
    }
    else if (token.is_string()) {
        auto string = UTF8::decode(token.as_string());
        switch (string.size()) {
            case 0:
                throw LocationException(token.location, "unicode character expected, got empty string");

            case 1:
                return string[0];

            default:
                throw LocationException(token.location, "unicode character expected, got multi-character string");
        }
    }
    else {
        throw LocationException(token.location, "invalid unicode character");
    }
}

uint8_t StringEncoding::get_uint8(Tokenizer& tokenizer) {
    return get_uint8(tokenizer.expect(Token::VALUE));
}

uint8_t StringEncoding::get_uint8(const Token& token) {
    if (token.is_unsigned()) {
        auto value = token.as_unsigned();
        if (value > std::numeric_limits<uint8_t>::max()) {
            throw LocationException(token.location, "character out of range");
        }
        return static_cast<uint8_t>(value);
    }
    else {
        throw LocationException(token.location, "invalid character");
    }
}

size_t StringEncoding::encoded_size(const Value& value) const {
    return encoded_size(value.string_value());
}

void StringEncoding::encode(std::string& bytes, const Value& value, std::optional<size_t> size) const {
    auto string = value.string_value();
    if (size) {
        if (string.size() < *size) {
            string.insert(string.size(), *size - string.size(), ' ');
        }
        else if (string.size() > *size) {
            throw Exception("value doesn't fit");
        }
    }
    return encode(bytes, string);
}

std::ostream& operator<<(std::ostream& stream, const StringEncoding& encoding) {
    encoding.serialize(stream);
    return stream;
}

void StringEncoding::serialize(std::ostream& stream) const {
    stream << name;
}