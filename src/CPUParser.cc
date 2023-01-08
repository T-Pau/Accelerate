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

Token::Group CPUParser::group_directive = Token::Group(Token::DIRECTIVE);
Token::Group CPUParser::group_newline = Token::Group(Token::NEWLINE);

SymbolTable CPUParser::directives;
std::map<symbol_t, void (CPUParser::*)()> CPUParser::parser_methods;


CPUParser::CPUParser() {
    if (directives.empty()) {
        parser_methods[directives.add("byte_order")] = &CPUParser::parse_byte_order;
        parser_methods[directives.add("addressing_mode")] = &CPUParser::parse_addressing_mode;
        parser_methods[directives.add("argument_type")] = &CPUParser::parse_argument_type;
        parser_methods[directives.add("include")] = &CPUParser::parse_include;
        parser_methods[directives.add("instruction")] = &CPUParser::parse_instruction;
    }
}


CPU CPUParser::parse(const std::string &file_name, FileReader& file_reader) {
    cpu = CPU();
    reader = &file_reader;
    tokenizer.push(file_name, file_reader.read(file_name));

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.expect(Token::DIRECTIVE, group_directive);
            auto it = parser_methods.find(directives[token.name]);
            if (it == parser_methods.end()) {
                throw ParseException(token.location, "unknown directive .%s", token.name.c_str());
            }
            (this->*it->second)();
        }
        catch (ParseException& ex) {
            reader->error(ex.location, "%s", ex.what());
        }
    }

    return std::move(cpu);
}

void CPUParser::parse_addressing_mode() {
    auto name_token = tokenizer.expect(Token::NAME, group_directive);
    if (cpu.addressing_mode_symbols.contains(name_token.name)) {
        tokenizer.skip_until(group_directive);
        throw ParseException(name_token.location, "addressing mode %s already defined", name_token.name.c_str());
    }
    auto name = cpu.addressing_mode_symbols.add(name_token.name);

    auto object = Object::parse(tokenizer);
    auto addressing_mode = AddressingMode();

    if (object->is_dictionary()) {
        throw ParseException(name_token, "addressing mode definition is not a dictionary");
    }

    auto definition = object->as_dictionary();

    auto arguments = (*definition)["arguments"];
    if (arguments != nullptr) {
        if (!arguments->is_dictionary()) {
            throw ParseException(name_token, "addressing mode definition is not a dictionary");
        }
        for (const auto& pair: (*arguments->as_dictionary())) {
            // TODO: parse argument definition
        }
    }

    // TODO: notations, encoding

    cpu.add_addressing_mode(name, addressing_mode);
}


void CPUParser::parse_argument_type() {

}

void CPUParser::parse_byte_order() {

}

void CPUParser::parse_include() {

}

void CPUParser::parse_instruction() {

}

bool CPUParser::parse_addressing_mode_line(AddressingMode &addressing_mode) {
    auto token = tokenizer.next();

    if (token.type == Token::CURLY_PARENTHESIS_CLOSE) {
        tokenizer.skip(Token::NEWLINE);
        return false;
    } else if (token.type != Token::NAME) {
        throw ParseException(token.location, "expected name, got %s", token.type_name());
    }

    tokenizer.expect(Token::COLON, group_newline);

    if (token.name == "notation") {
        auto tokens = tokenizer.collect_until(Token::NEWLINE);
        if (tokens.size() == 1 && tokens[0].type == Token::SQUARE_PARENTHESIS_OPEN) {
            // TODO: multiple notations
        }
        addressing_mode.add_notation(tokens);
    }
    else if (token.name == "arguments") {

    }
    else if (token.name == "encoding") {

    }
    else {
        tokenizer.skip_until(group_newline);
        throw ParseException(token.location, "unknown addressing mode field %s", token.name.c_str());
    }
    return true;
}

