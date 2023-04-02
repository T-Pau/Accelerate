/*
CPUParser.h -- Parse CPU Specification

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

#ifndef CPU_PARSER_H
#define CPU_PARSER_H

#include "CPU.h"
#include "FileReader.h"
#include "Symbol.h"
#include "TokenizerFile.h"
#include "FileParser.h"

class CPUParser: public FileParser {
public:
    CPUParser();

    CPU parse(Symbol file_name);

protected:
    void parse_directive(const Token& directive) override;

private:
    void parse_addressing_mode();
    void parse_argument_type();
    void parse_byte_order();
    void parse_instruction();
    void parse_syntax();

    static Symbol argument_symbol(Symbol name);

    AddressingMode::Notation parse_addressing_mode_notation(const AddressingMode& addressing_mode, const ParsedScalar* parameters);

    std::unique_ptr<ArgumentType> parse_argument_type_enum(const Token& name, const ParsedValue* parameters);
    std::unique_ptr<ArgumentType> parse_argument_type_map(const Token& name, const ParsedValue* parameters);
    std::unique_ptr<ArgumentType> parse_argument_type_range(const Token& name, const ParsedValue* parameters);

    static std::unordered_map<Symbol, std::unique_ptr<ArgumentType> (CPUParser::*)(const Token& name, const ParsedValue* parameters)> argument_type_parser_methods;
    static std::unordered_map<Symbol, void (CPUParser::*)()> parser_methods;

    CPU cpu;

    std::unordered_set<Token> addressing_mode_names;
    std::unordered_set<Token> argument_type_names;

    static void initialize();
    static bool initialized;
    static Token token_arguments;
    static Token token_comma;
    static Token token_encoding;
    static Token token_keywords;
    static Token token_minus;
    static Token token_notation;
    static Token token_opcode;
    static Token token_pc;
    static Token token_punctuation;
};


#endif // CPU_PARSER_H
