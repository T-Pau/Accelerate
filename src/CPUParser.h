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
#include "FileReader.h"
#include "SymbolTable.h"
#include "Tokenizer.h"

class CPUParser {
public:
    CPUParser();

    CPU parse(const std::string& file_name, FileReader& file_reader);

private:
    void parse_addressing_mode(const Token& name, const std::shared_ptr<Object>& parameters);
    void parse_argument_type(const Token& name, const std::shared_ptr<Object>& parameters);
    void parse_byte_order(const Token& name, const std::shared_ptr<Object>& parameters);
    void parse_instruction(const Token& name, const std::shared_ptr<Object>& parameters);

    static Token::Group group_directive;
    static Token::Group group_newline;

    static std::map<symbol_t, void (CPUParser::*)(const Token& name, const std::shared_ptr<Object>& parameters)> parser_methods;
    static symbol_t symbol_byte_order;

    FileReader* reader = nullptr;
    Tokenizer tokenizer;
    CPU cpu;
};


#endif // CPU_PARSER_H
