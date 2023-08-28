#ifndef STRING_ENCODING_H
#define STRING_ENCODING_H

/*
StringEncoding.h --

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

#include <string>
#include <unordered_map>

#include "ParsedValue.h"
#include "Symbol.h"

class Target;

class StringEncoding {
  public:
    StringEncoding() = default;
    StringEncoding(Symbol name, const std::shared_ptr<ParsedValue>& definition, const Target &target);

    [[nodiscard]] size_t encoded_size(const std::string& string) const {return encode(nullptr, string);}
    [[nodiscard]] size_t encoded_size(const Value& value) const;
    void encode(std::string& bytes, const std::string& string) const {(void)encode(&bytes, string);}
    void encode(std::string& bytes, const Value& value) const;

    void serialize(std::ostream& stream) const;

  private:
    class CharacterRange {
      public:
        CharacterRange(char32_t source_start, uint8_t target_start, size_t length): source_start(source_start), target_start(target_start), length(length) {}

        [[nodiscard]] uint8_t target_end() const {return target_start + length - 1;}
        [[nodiscard]] char32_t source_end() const {return source_start + length - 1;}

        [[nodiscard]] std::optional<uint8_t> encode(char32_t codepoint) const;

        [[nodiscard]] bool operator<(const CharacterRange& other) const {return source_start < other.source_start || (source_start == other.source_start && length < other.length);}
        [[nodiscard]] bool overlaps(const CharacterRange& other) const {return source_end() >= other.source_start && source_end() <= other.source_end();}

        char32_t source_start;
        uint8_t target_start;
        size_t length;
    };

    size_t encode(std::string* bytes, const std::string& string) const;
    [[nodiscard]] std::optional<uint8_t> encode(char32_t codepoint) const;

    void add_range(const std::shared_ptr<ParsedValue>& range);
    void add_range(const CharacterRange& range) {ranges.emplace_back(range);}
    void add_named(const Token& target_token, const std::shared_ptr<ParsedValue>& sources);
    void add_named(const std::u32string& source, uint8_t target);
    void add_named_ranges(const Token& prefix_token, const std::shared_ptr<ParsedValue>& ranges);
    void add_named_range(const std::u32string& prefix, char32_t source_start, uint8_t target_start, uint8_t length);
    void add_singleton(const Token& target_token, const std::shared_ptr<ParsedValue>& sources);
    void add_singleton(uint32_t source, uint8_t target);
    void import_base(const Token& base_name, const std::shared_ptr<ParsedValue>& base_parameters, const Target& target);
    void import_base_range(const StringEncoding* base, uint8_t start, uint8_t end, uint8_t offset);
    void set_name_delimiters(const std::shared_ptr<ParsedValue>& delimiters);

#if 0
    [[nodiscard]] static std::u32string decode_utf8(const std::string& string);
    [[nodiscard]] static std::string encode_utf8(const std::u32string& string);
    [[nodiscard]] static std::string encode_utf8(char32_t codepoint);
#endif

    [[nodiscard]] static char32_t get_char32(const Token& token);
    [[nodiscard]] static char32_t get_char32(Tokenizer& tokenizer);
    [[nodiscard]] static uint8_t get_uint8(const Token& token);
    [[nodiscard]] static uint8_t get_uint8(Tokenizer& tokenizer);
    [[nodiscard]] static bool is_in_range(uint8_t value, uint8_t start, uint8_t end) {return value >= start && value <= end;}

    std::string name;

    std::vector<CharacterRange> ranges;
    std::unordered_map<char32_t, uint8_t> singletons;
    std::u32string named_open;
    std::u32string named_close;
    std::unordered_map<std::u32string, uint8_t> named;

    static const Token token_base;
    static const Token token_name_delimiters;
    static const Token token_named;
    static const Token token_named_ranges;
    static const Token token_ranges;
    static const Token token_singletons;
    static const Token token_use;
    static const TokenGroup group_char32;
};

std::ostream& operator<<(std::ostream& stream, const StringEncoding& encoding);

#endif // STRING_ENCODING_H
