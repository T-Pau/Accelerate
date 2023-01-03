/*
CpuParser.cc -- Parse CPU Specification

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

#include "CPUParser.h"
#include "Tokenizer.h"
#include "Exception.h"


SymbolTable CPUParser::directives;
std::map<symbol_t, void (CPUParser::*)()> CPUParser::parser_methods;


CPUParser::CPUParser() {
    if (directives.empty()) {
        parser_methods[directives.add("byte_order")] = &CPUParser::parse_byte_order;
        parser_methods[directives.add("addressing_modes")] = &CPUParser::parse_addressing_modes;
        parser_methods[directives.add("argument_types")] = &CPUParser::parse_argument_types;
        parser_methods[directives.add("include")] = &CPUParser::parse_include;
        parser_methods[directives.add("instructions")] = &CPUParser::parse_instructions;
    }
}


CPU CPUParser::parse(const std::string &file_name, FileReader& file_reader) {
    cpu = CPU();
    tokenizer.push(file_name, file_reader.read(file_name));

    while (auto token = tokenizer.next()) {
        if (token.type == Token::DIRECTIVE) {
            auto method = parser_methods.find(directives[token.name])->second;
            (this->*method)();
        }
    }

    return std::move(cpu);
}

void CPUParser::parse_addressing_mode(const std::string &name) {
    if (cpu.addressing_mode_symbols.contains(name)) {
        throw Exception("duplicate addressing_mode '" + name + "'");
    }

    while (auto token = tokenizer.next()) {

    }
}

void CPUParser::parse_argument_type(const std::string &name) {

}

void CPUParser::parse_byte_order() {

}

void CPUParser::parse_include() {

}

void CPUParser::parse_instruction(const std::string &name) {

}

void CPUParser::parse_map(void (CPUParser::*parse_element)(const std::string &)) {
    std::string name;
    while (auto token = tokenizer.next()) {
        if (token.is_directive()) {
            tokenizer.unget(token);
            return;
        }
        else if (token.is_newline()) {
            continue;
        }
        else if (!token.is_name()) {

        }
    }
}
