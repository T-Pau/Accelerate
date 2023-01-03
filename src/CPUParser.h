/*
CPUParser.h -- Parse CPU Specification

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

#ifndef CPU_PARSER_H
#define CPU_PARSER_H

#include "CPU.h"
#include "SymbolTable.h"
#include "FileReader.h"
#include "Tokenizer.h"

class CPUParser {
public:
    CPUParser();

    CPU parse(const std::string& file_name, FileReader& file_reader);

private:
    void parse_addressing_mode(const std::string& name);
    void parse_addressing_modes() {parse_map(&CPUParser::parse_addressing_mode);}
    void parse_argument_type(const std::string& name);
    void parse_argument_types() {parse_map(&CPUParser::parse_argument_type);}
    void parse_byte_order();
    void parse_include();
    void parse_instruction(const std::string& name);
    void parse_instructions() {parse_map(&CPUParser::parse_instruction);}
    void parse_map(void (CPUParser::*parse_element)(const std::string& name));

    static SymbolTable directives;
    static std::map<symbol_t, void (CPUParser::*)()> parser_methods;

    Tokenizer tokenizer;
    CPU cpu;
};


#endif // CPU_PARSER_H
