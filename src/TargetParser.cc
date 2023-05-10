/*
TargetParser.cc --

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

#include "TargetParser.h"

#include "Exception.h"
#include "ParseException.h"
#include "FileReader.h"
#include "ExpressionParser.h"
#include "TargetGetter.h"
#include "BodyParser.h"

bool TargetParser::initialized = false;
std::unordered_map<Symbol, void (TargetParser::*)()> TargetParser::parser_methods;
Token TargetParser::token_address;
Token TargetParser::token_colon;
Token TargetParser::token_cpu;
Token TargetParser::token_data;
Token TargetParser::token_data_end;
Token TargetParser::token_data_size;
Token TargetParser::token_data_start;
Token TargetParser::token_extension;
Token TargetParser::token_memory;
Token TargetParser::token_minus;
Token TargetParser::token_output;
Token TargetParser::token_read_only;
Token TargetParser::token_read_write;
Token TargetParser::token_reserve_only;
Token TargetParser::token_section;
Token TargetParser::token_segment;
Token TargetParser::token_segment_name;
Token TargetParser::token_type;


TargetParser::TargetParser(): FileParser(TargetGetter::global.path) {
    initialize();

    tokenizer.add_literal(token_data_end);
    tokenizer.add_literal(token_data_size);
    tokenizer.add_literal(token_data_start);
}


void TargetParser::initialize() {
    if (!initialized) {
        token_address = Token(Token::NAME, "address");
        token_colon = Token(Token::PUNCTUATION, ":");
        token_cpu = Token(Token::DIRECTIVE, "cpu");
        token_data = Token(Token::DIRECTIVE, "data");
        token_data_end = Token(Token::NAME, ".data_end");
        token_data_size = Token(Token::NAME, ".data_size");
        token_data_start = Token(Token::NAME, ".data_start");
        token_extension = Token(Token::DIRECTIVE, "extension");
        token_memory = Token(Token::DIRECTIVE, "memory");
        token_minus = Token(Token::PUNCTUATION, "-");
        token_output = Token(Token::DIRECTIVE, "output");
        token_read_only = Token(Token::NAME, "read_only");
        token_read_write = Token(Token::NAME, "read_write");
        token_reserve_only = Token(Token::NAME, "reserve_only");
        token_section = Token(Token::DIRECTIVE, "section");
        token_segment = Token(Token::DIRECTIVE, "segment");
        token_segment_name = Token(Token::NAME, "segment");
        token_type = Token(Token::NAME, "type");

        parser_methods[token_cpu.as_symbol()] = &TargetParser::parse_cpu;
        parser_methods[token_extension.as_symbol()] = &TargetParser::parse_extension;
        parser_methods[token_output.as_symbol()] = &TargetParser::parse_output;
        parser_methods[token_section.as_symbol()] = &TargetParser::parse_section;
        parser_methods[token_segment.as_symbol()] = &TargetParser::parse_segment;

        initialized = true;
    }
}


Target TargetParser::parse(Symbol name, Symbol file_name) {
    target = Target();
    target.name = name;
    had_cpu = false;
    section_names.clear();
    segment_names.clear();

    if (!parse_file(file_name)) {
        throw Exception("can't parse target file '%s'", file_name.c_str());
    }

    if (!had_cpu) {
        throw ParseException(Location(), "missing CPU declaration");
    }
    return std::move(target);
}


void TargetParser::parse_directive(const Token &directive) {
    auto it = parser_methods.find(directive.as_symbol());
    if (it == parser_methods.end()) {
        throw ParseException(directive, "unknown directive");
    }
    (this->*it->second)();
}


void TargetParser::parse_output() {
    auto token = tokenizer.next();
    if (token != Token::curly_open) {
        tokenizer.skip_until(TokenGroup::newline, true);
        throw ParseException(token, "expected '{'");
    }

    target.output = BodyParser(tokenizer, target.cpu).parse();
}


void TargetParser::parse_section() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parse_value = ParsedValue::parse(tokenizer);
    auto parameters = parse_value->as_dictionary();

    if (section_names.contains(name)) {
        throw ParseException(name, "duplicate section definition"); // TODO: attach note
    }
    section_names.insert(name);

    auto type = MemoryMap::READ_WRITE;

    auto type_parameter = parameters->get_optional(token_type);
    if (type_parameter != nullptr) {
        type = parse_type(type_parameter->as_singular_scalar()->token());
    }
    auto blocks = std::vector<MemoryMap::Block>();
    auto segment = parameters->get_optional(token_segment_name);
    if (segment) {
        if (parameters->has_key(token_address)) {
            throw ParseException(name, "segment and address are mutually exclusive");
        }

        auto segment_blocks = target.map.segment(segment->as_singular_scalar()->token().as_symbol());
        if (segment_blocks == nullptr) {
            throw ParseException(segment->as_singular_scalar()->token(), "unknown segment");
        }
        blocks.insert(blocks.begin(), segment_blocks->begin(), segment_blocks->end());
    }
    else {
        blocks = parse_address((*parameters)[token_address].get());
    }

    target.map.add_section(MemoryMap::Section(name.as_symbol(), type, std::move(blocks)));
}


void TargetParser::parse_segment() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parse_value = ParsedValue::parse(tokenizer);
    auto parameters = parse_value->as_dictionary();

    if (segment_names.contains(name)) {
        throw ParseException(name, "duplicate segment definition"); // TODO: attach note
    }
    segment_names.insert(name);

    target.map.add_segment(name.as_symbol(), parse_address((*parameters)[token_address].get()));
}


std::vector<MemoryMap::Block> TargetParser::parse_address(const ParsedValue *address) {
    auto blocks = std::vector<MemoryMap::Block>();

    if (address->is_array()) {
        for (const auto& element: *address->as_array()) {
            blocks.emplace_back(parse_single_address(element->as_scalar()));
        }
    }
    else {
        blocks.emplace_back(parse_single_address(address->as_scalar()));
    }

    return blocks;
}


MemoryMap::AccessType TargetParser::parse_type(Token type) {
    if (type == token_reserve_only) {
        return MemoryMap::RESERVE_ONLY;
    }
    else if (type == token_read_only) {
        return MemoryMap::READ_ONLY;
    }
    else if (type == token_read_write) {
        return MemoryMap::READ_WRITE;
    }
    else {
        throw ParseException(type, "invalid type");
    }
}

MemoryMap::Block TargetParser::parse_single_address(const ParsedScalar *address) {
    size_t index = 0;
    uint64_t bank = 0;

    if (address->size() > 2 && (*address)[1] == token_colon) {
        auto bank_token = (*address)[0];

        if (!bank_token.is_unsigned()) {
            throw ParseException(bank_token, "unsigned integer expected");
        }
        bank = bank_token.as_unsigned();
        index = 2;
    }
    if (address->size() < index + 1) {
        throw ParseException(address->location, "missing address");
    }
    auto start_token = (*address)[index];
    if (!start_token.is_unsigned()) {
        throw ParseException(start_token, "unsigned integer expected");
    }
    auto start = start_token.as_unsigned();
    uint64_t size = 1;

    if (address->size() == index + 3 && (*address)[index + 1] == token_minus) {
        auto end_token = (*address)[index + 2];
        if (!end_token.is_unsigned()) {
            throw ParseException(start_token, "unsigned integer expected");
        }
        size = end_token.as_unsigned() - start + 1;
    }
    else if (address->size() != index + 1) {
        throw ParseException(address->location, "invalid address specification");
    }

    return {bank, start, size};
}

void TargetParser::parse_cpu() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);

    try {
        had_cpu = true;
        target.cpu = &CPU::get(name.as_symbol(), tokenizer.current_file());
    }
    catch (Exception &ex) {
        throw ParseException(name, "%s", ex.what());
    }
}

void TargetParser::parse_extension() {
    auto token = tokenizer.expect(Token::STRING, TokenGroup::newline);

    target.extension = token.as_string();
}
