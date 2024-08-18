/*
Assembler.h --

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

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "ObjectFile.h"
#include "Target.h"
#include "FileTokenizer.h"
#include "Object.h"

class Assembler {
public:
    explicit Assembler(const Target* target, const Path& path, const std::unordered_set<Symbol>& defines);

    Target parse_target(Symbol name, Symbol file_name);
    std::shared_ptr<ObjectFile> parse_object_file(Symbol file_name);

    static const Symbol symbol_opcode;
    static const Token token_data_end;
    static const Token token_data_start;
    static const Token token_data_size;

private:
    enum SectionDefinitionType {
        SECTION_DEFINE,
        SECTION_EXTEND,
        SECTION_OVERRIDE
    };
    class Directive {
    public:
        Directive(void (Assembler::*parse)(const Token& directive), bool target_only = false) : parse{parse}, target_only{target_only} {}

        void (Assembler::*parse)(const Token& directive);
        bool target_only;
    };

    void parse(Symbol file_name);
    void parse_assignment(Visibility visibility, const Token& name, bool default_only = false);
    void parse_cpu(const Token& directive);
    void parse_default(const Token& directive);
    void parse_default_string_encoding(const Token& directive);
    void parse_directive(const Token& directive);
    void parse_extension(const Token& directive);
    void parse_macro(Visibility visibility, bool default_only = false);
    void parse_name(Visibility visibility, const Token& name, bool default_only = false);
    void parse_output(const Token& directive);
    void parse_pin(const Token& directive);
    void parse_section(const Token& directive);
    void parse_segment(const Token& directive);
    void parse_string_encoding(const Token& directive);
    void parse_symbol(Visibility visibility, const Token& name);
    void parse_target(const Token& directive);
    void parse_use(const Token& directive);
    void parse_visibility(const Token& directive);
    void set_target(const Target* new_target);

    std::vector<MemoryMap::Block> parse_address(const ParsedValue* address) const;
    MemoryMap::Block parse_single_address(const ParsedScalar* address) const;
    uint64_t parse_address_part(const Token& token) const;
    static MemoryMap::AccessType parse_type(const Token& type);

    const Target* target{};
    const CPU *cpu{};

    Symbol current_section;
    Visibility current_visibility{Visibility::PRIVATE};
    std::unordered_set<Token> section_names;
    std::unordered_set<Token> segment_names;
    bool had_cpu{false};

    bool parsing_target{};
    FileTokenizer tokenizer;
    Target parsed_target;
    std::shared_ptr<ObjectFile> object_file;

    static const Token token_address;
    static const Token token_address_name;
    static const Token token_align;
    static const Token token_cpu;
    static const Token token_default;
    static const Token token_default_string_encoding;
    static const Token token_extend;
    static const Token token_extension;
    static const Token token_macro;
    static const Token token_output;
    static const Token token_override;
    static const Token token_pin;
    static const Token token_read_only;
    static const Token token_read_write;
    static const Token token_reserve;
    static const Token token_reserve_only;
    static const Token token_section;
    static const Token token_segment;
    static const Token token_segment_name;
    static const Token token_string_encoding;
    static const Token token_target;
    static const Token token_type;
    static const Token token_use;
    static const Token token_used;
    static const Token token_uses;
    static const Token token_visibility;
    static const std::unordered_map<Token, Directive> directives;
};

#endif // ASSEMBLER_H
