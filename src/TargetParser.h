/*
TargetParser.h --

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

#ifndef TARGET_PARSER_H
#define TARGET_PARSER_H


#include "FileParser.h"
#include "ParsedValue.h"
#include "Target.h"

class TargetParser: public FileParser {
public:
    TargetParser();

    Target parse(Symbol name, Symbol file_name);

    static const Token token_data_end;
    static const Token token_data_start;
    static const Token token_data_size;

protected:
    void parse_directive(const Token& directive) override;

private:
    void parse_cpu(const Token& directive);
    void parse_default_string_encoding(const Token& directive);
    void parse_define(const Token& directive);
    void parse_extension(const Token& directive);
    void parse_output(const Token& directive);
    void parse_section(const Token& directive);
    void parse_segment(const Token& directive);
    void parse_string_encoding(const Token& directive);
    void parse_undefine(const Token& directive);

    static std::vector<MemoryMap::Block> parse_address(const ParsedValue* address);
    static MemoryMap::Block parse_single_address(const ParsedScalar* address);
    static MemoryMap::AccessType parse_type(const Token& type);

    Target target;
    std::unordered_set<Token> section_names;
    std::unordered_set<Token> segment_names;
    bool had_cpu = false;

    static void initialize();
    static bool initialized;
    static std::unordered_map<Symbol, void (TargetParser::*)(const Token& directive)> parser_methods;
    static const Token token_address;
    static const Token token_colon;
    static const Token token_cpu;
    static const Token token_data;
    static const Token token_default_string_encoding;
    static const Token token_define;
    static const Token token_extension;
    static const Token token_memory;
    static const Token token_output;
    static const Token token_read_only;
    static const Token token_read_write;
    static const Token token_reserve_only;
    static const Token token_section;
    static const Token token_segment;
    static const Token token_segment_name;
    static const Token token_string_encoding;
    static const Token token_type;
    static const Token token_undefine;
};


#endif // TARGET_PARSER_H
