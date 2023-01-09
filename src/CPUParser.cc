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
#include "ParseException.h"
#include "AddressingMode.h"
#include "Object.h"

Token::Group CPUParser::group_directive = Token::Group({Token::DIRECTIVE,Token::END}, "directive");
Token::Group CPUParser::group_newline = Token::Group(Token::NEWLINE);

std::map<symbol_t, void (CPUParser::*)(const Token& name, const std::shared_ptr<Object>& parameters)> CPUParser::parser_methods;
symbol_t CPUParser::symbol_byte_order;

CPUParser::CPUParser() {
    if (parser_methods.empty()) {
        symbol_byte_order = SymbolTable::global.add("byte_order");
        parser_methods[symbol_byte_order] = &CPUParser::parse_byte_order;
        parser_methods[SymbolTable::global.add("addressing_mode")] = &CPUParser::parse_addressing_mode;
        parser_methods[SymbolTable::global.add("argument_type")] = &CPUParser::parse_argument_type;
        parser_methods[SymbolTable::global.add("instruction")] = &CPUParser::parse_instruction;
    }
}


CPU CPUParser::parse(const std::string &file_name, FileReader& file_reader) {
    cpu = CPU();
    reader = &file_reader;
    tokenizer.push(file_name, file_reader.read(file_name));

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.expect(group_directive, group_directive);
            if (!token) {
                break;
            }
            auto it = parser_methods.find(token.as_symbol());
            if (it == parser_methods.end()) {
                throw ParseException(token.location, "unknown directive .%s", token.as_string().c_str());
            }

            Token name;
            if (token.as_symbol() != symbol_byte_order) {
                name = tokenizer.expect(Token::NAME, group_directive);
            }
            auto parameters = Object::parse(tokenizer);

            (this->*it->second)(name, parameters);
        }
        catch (ParseException& ex) {
            reader->error(ex.location, "%s", ex.what());
            tokenizer.skip_until(group_directive);
        }
    }

    return std::move(cpu);
}

void CPUParser::parse_addressing_mode(const Token& name, const std::shared_ptr<Object>& parameters) {
    if (cpu.addressing_modes.find(name.as_symbol()) != cpu.addressing_modes.end()) {
        tokenizer.skip_until(group_directive);
        throw ParseException(name.location, "addressing mode %s already defined", name.as_string().c_str());
    }

    if (!parameters->is_dictionary()) {
        throw ParseException(name, "addressing mode definition is not a dictionary");
    }

    auto addressing_mode = AddressingMode();

    auto definition = parameters->as_dictionary();

    auto arguments = (*definition)["arguments"];
    if (arguments != nullptr) {
        if (!arguments->is_dictionary()) {
            throw ParseException(name, "addressing mode definition is not a dictionary");
        }
        for (const auto& pair: (*arguments->as_dictionary())) {
            // TODO: parse argument definition
        }
    }

    // TODO: notations, encoding

    cpu.add_addressing_mode(name.as_symbol(), addressing_mode);
}


void CPUParser::parse_argument_type(const Token& name, const std::shared_ptr<Object>& parameters) {

}

void CPUParser::parse_byte_order(const Token& name, const std::shared_ptr<Object>& parameters) {

}

void CPUParser::parse_instruction(const Token& name, const std::shared_ptr<Object>& parameters) {

}
