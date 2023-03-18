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

    Target parse(const std::string& file_name);

protected:
    void parse_directive(const Token& directive) override;

private:
    void parse_cpu();
    void parse_extension();
    void parse_output();
    void parse_section();
    void parse_segment();

    static std::vector<MemoryMap::Block> parse_address(const ParsedValue* address);
    static MemoryMap::Block parse_single_address(const ParsedScalar* address);
    static MemoryMap::AccessType parse_type(Token type);

    Target target;
    std::unordered_set<Token> section_names;
    std::unordered_set<Token> segment_names;
    bool had_cpu = false;

    static void initialize();
    static bool initialized;
    static std::unordered_map<symbol_t, void (TargetParser::*)()> parser_methods;
    static Token token_address;
    static Token token_colon;
    static Token token_cpu;
    static Token token_data;
    static Token token_extension;
    static Token token_memory;
    static Token token_minus;
    static Token token_output;
    static Token token_read_only;
    static Token token_read_write;
    static Token token_reserve_only;
    static Token token_section;
    static Token token_segment;
    static Token token_segment_name;
    static Token token_type;
};


#endif // TARGET_PARSER_H
