/*
CpuParser.cc -- Parse CPU Specification

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

#include "CPUParser.h"
#include "TokenizerFile.h"
#include "Exception.h"
#include "ParseException.h"
#include "AddressingMode.h"
#include "ParsedValue.h"
#include "TokenizerSequence.h"
#include "ExpressionParser.h"
#include "VariableExpression.h"


std::unordered_map<symbol_t, std::unique_ptr<ArgumentType> (CPUParser::*)(const Token& name, const ParsedValue* parameters)> CPUParser::argument_type_parser_methods;
std::unordered_map<symbol_t, void (CPUParser::*)()> CPUParser::parser_methods;

bool CPUParser::initialized = false;
Token CPUParser::token_arguments;
Token CPUParser::token_comma;
Token CPUParser::token_encoding;
Token CPUParser::token_keywords;
Token CPUParser::token_minus;
Token CPUParser::token_notation;
Token CPUParser::token_opcode;
Token CPUParser::token_pc;
Token CPUParser::token_punctuation;

void CPUParser::initialize() {
    if (!initialized) {
        token_arguments = Token(Token::NAME, {}, "arguments");
        token_comma = Token(Token::PUNCTUATION, {}, ",");
        token_encoding = Token(Token::NAME, {}, "encoding");
        token_keywords = Token(Token::NAME, {}, "keywords");
        token_minus = Token(Token::PUNCTUATION, {}, "-");
        token_notation = Token(Token::NAME, {}, "notation");
        token_opcode = Token(Token::NAME, {}, ".opcode");
        token_pc = Token(Token::NAME, {}, ".pc");
        token_punctuation = Token(Token::NAME, {}, "punctuation");

        parser_methods[SymbolTable::global.add("byte_order")] = &CPUParser::parse_byte_order;
        parser_methods[SymbolTable::global.add("addressing_mode")] = &CPUParser::parse_addressing_mode;
        parser_methods[SymbolTable::global.add("argument_type")] = &CPUParser::parse_argument_type;
        parser_methods[SymbolTable::global.add("instruction")] = &CPUParser::parse_instruction;
        parser_methods[SymbolTable::global.add("syntax")] = &CPUParser::parse_syntax;

        argument_type_parser_methods[SymbolTable::global.add("enum")] = &CPUParser::parse_argument_type_enum;
        argument_type_parser_methods[SymbolTable::global.add("map")] = &CPUParser::parse_argument_type_map;
        argument_type_parser_methods[SymbolTable::global.add("range")] = &CPUParser::parse_argument_type_range;

        initialized = true;
    }
}

CPUParser::CPUParser() {
    initialize();

    tokenizer.add_punctuations({"-", ","});
    tokenizer.add_literal(token_opcode);
    tokenizer.add_literal(token_pc);
}


CPU CPUParser::parse(const std::string &file_name) {
    cpu = CPU();

    if (!parse_file(file_name)) {
        throw Exception("can't parse object file '%s'", file_name.c_str());
    }
    return std::move(cpu);
}

void CPUParser::parse_directive(const Token &directive) {
    auto it = parser_methods.find(directive.as_symbol());
    if (it == parser_methods.end()) {
        throw ParseException(directive, "unknown directive");
    }
    (this->*it->second)();
}

void CPUParser::parse_addressing_mode() {
    Token name = tokenizer.expect(Token::NAME, group_directive);
    auto parameters = ParsedValue::parse(tokenizer);

    {
        auto it = addressing_mode_names.find(name);
        if (it != addressing_mode_names.end()) {
            throw ParseException(name.location, "duplicate definition of addressing mode '%s'", name.as_string().c_str());
            // TODO: attach note it->second,  previously defined here
        }
        addressing_mode_names.insert(name);
    }

    if (!parameters->is_dictionary()) {
        throw ParseException(name, "addressing mode definition is not a dictionary");
    }

    auto addressing_mode = AddressingMode();

    auto definition = parameters->as_dictionary();

    auto arguments = definition->get_optional(token_arguments);
    if (arguments != nullptr) {
        if (!arguments->is_dictionary()) {
            throw ParseException(name, "addressing mode definition is not a dictionary");
        }
        for (const auto& pair: (*arguments->as_dictionary())) {
            if (!pair.first.is_name()) {
                throw ParseException(pair.first, "expected %s, got %s", Token::type_name(Token::NAME), pair.first.type_name());
            }
            if (!pair.second->is_singular_scalar()) {
                throw ParseException(pair.second->location, "invalid argument type");
            }
            auto argument_type_name = pair.second->as_scalar()->token();
            if (!argument_type_name.is_name()) {
                throw ParseException(argument_type_name, "expected %s, got %s", Token::type_name(Token::NAME), argument_type_name.type_name());
            }
            auto argument_type = cpu.argument_type(argument_type_name.as_symbol());
            if (argument_type == nullptr) {
                throw ParseException(argument_type_name, "unknown argument type '%s'", argument_type_name.as_string().c_str());
            }
            addressing_mode.arguments[argument_symbol(pair.first.as_symbol())] = argument_type;
        }
    }

    auto notation = (*definition)[token_notation];
    if (notation == nullptr) {
        throw ParseException(name, "notation missing for addressing mode '%s'", name.as_string().c_str());
    }
    else if (notation->is_scalar()) {
        addressing_mode.add_notation(parse_addressing_mode_notation(addressing_mode, notation->as_scalar()));
    }
    else if (notation->is_array()) {
        for (const auto& n: (*notation->as_array())) {
            if (!n->is_scalar()) {
                throw ParseException(notation->location, "invalid notation for addressing mode '%s'", name.as_string().c_str());
            }
            addressing_mode.add_notation(parse_addressing_mode_notation(addressing_mode, n->as_scalar()));
        }
    }
    else {
        throw ParseException(notation->location, "invalid notation for addressing mode '%s'", name.as_string().c_str());
    }

    auto encoding_definition = definition->get_optional(token_encoding);
    if (encoding_definition == nullptr) {
        addressing_mode.encoding = {std::make_shared<VariableExpression>(token_opcode)};
    }
    else if (encoding_definition->is_scalar()) {
        auto encoding_tokenizer = TokenizerSequence(encoding_definition->as_scalar()->tokens);
        auto encoding = ExpressionParser(encoding_tokenizer).parse_list();

        for (auto& expression: encoding) {
            expression->replace_variables(argument_symbol);
        }
        addressing_mode.encoding = encoding;
    }
    else {
        throw ParseException(name, "invalid encoding for addressing mode '%s'", name.as_string().c_str());
    }

    cpu.add_addressing_mode(name.as_symbol(), addressing_mode);
}


void CPUParser::parse_argument_type() {
    Token name = tokenizer.expect(Token::NAME, group_directive);
    Token type = tokenizer.expect(Token::NAME, group_directive);
    auto parameters = ParsedValue::parse(tokenizer);

    {
        auto it = argument_type_names.find(name);
        if (it != argument_type_names.end()) {
            throw ParseException(type, "duplicate definition of argument type '%s'", name.as_string().c_str());
            // TODO: attach note it->second,  previously defined here
        }
        argument_type_names.insert(name);
    }

    auto it = argument_type_parser_methods.find(type.as_symbol());
    if (it == argument_type_parser_methods.end()) {
        throw ParseException(type, "unknown argument type '%s'", type.as_string().c_str());
    }
    auto argument_type = (this->*it->second)(name, parameters.get());

    cpu.add_argument_type(name.as_symbol(), std::move(argument_type));
}

void CPUParser::parse_byte_order() {
    Token byte_order = tokenizer.expect(Token::INTEGER, group_directive);
    tokenizer.skip(Token::NEWLINE);

    cpu.byte_order = byte_order.as_integer();
}


void CPUParser::parse_instruction() {
    Token name = tokenizer.expect(Token::NAME, group_directive);
    auto parameters = ParsedValue::parse(tokenizer);

    if (!parameters->is_dictionary()) {
        throw ParseException(parameters->location, "instruction definition must be dictionary");
    }

    auto& instruction = cpu.instructions[name.as_symbol()];

    for (const auto& pair: (*parameters->as_dictionary())) {
        if (!pair.first.is_name()) {
            throw ParseException(pair.first, "addressing mode must be name");
        }
        if (addressing_mode_names.find(pair.first) == addressing_mode_names.end()) {
            throw ParseException(pair.first, "unknown addressing mode");
        }
        auto it = instruction.opcodes.find(pair.first.as_symbol());
        if (it != instruction.opcodes.end()) {
            throw ParseException(pair.first, "redefinition of addressing mode");
        }
        if (!pair.second->is_singular_scalar() || !pair.second->as_scalar()->token().is_integer()) {
            throw ParseException(pair.second->location, "opcode must be integer");
        }
        instruction.opcodes[pair.first.as_symbol()] = pair.second->as_scalar()->token().as_integer();
    }
}


void CPUParser::parse_syntax() {
    auto type = tokenizer.expect(Token::NAME, group_directive);
    auto values = ParsedValue::parse(tokenizer);

    if (!values->is_scalar()) {
        throw ParseException(values->location, "expected strings");
    }

    if (type == token_keywords) {
        for (const auto& value : (*values->as_scalar())) {
            if (!value.is_string()) {
                throw ParseException(value, "expected string");
            }
            cpu.add_reserved_word(SymbolTable::global.add(value.as_string()));
            tokenizer.add_literal(Token::NAME, value.as_string());
        }
    }
    else if (type == token_punctuation) {
        for (const auto& value : (*values->as_scalar())) {
            if (!value.is_string()) {
                throw ParseException(value, "expected string");
            }
            cpu.add_punctuation(SymbolTable::global.add(value.as_string()));
            tokenizer.add_literal(Token::PUNCTUATION, value.as_string());
        }
    }
}


std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_enum(const Token& name, const ParsedValue *parameters) {
    auto argument_type = std::make_unique<ArgumentTypeEnum>();

    if (!parameters->is_dictionary()) {
        throw ParseException(parameters->location, "definition of enum argument type '%s' must be dictionary", name.as_string().c_str());
    }

    for (const auto& pair: (*parameters->as_dictionary())) {
        if (!pair.first.is_name()) {
            throw ParseException(pair.first, "key for enum argument type '%s' must be name", name.as_string().c_str());
        }
        if (!pair.second->is_singular_scalar()) {
            throw ParseException(pair.first, "key for enum argument type '%s' must be single scalar", name.as_string().c_str());
        }
        const auto& value = pair.second->as_scalar()->token();
        if (!value.is_integer()) {
            throw ParseException(pair.first, "key for enum argument type '%s' must be integer", name.as_string().c_str());
        }
        auto symbol = pair.first.as_symbol();
        cpu.add_reserved_word(symbol);
        argument_type->entries[symbol] = value.as_integer();
    }

    return argument_type;
}

std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_map(const Token& name, const ParsedValue *parameters) {
    auto argument_type = std::make_unique<ArgumentTypeMap>();

    if (!parameters->is_dictionary()) {
        throw ParseException(parameters->location, "definition of map argument type '%s' must be dictionary", name.as_string().c_str());
    }

    for (const auto& pair: (*parameters->as_dictionary())) {
        if (!pair.first.is_integer()) {
            throw ParseException(pair.first, "key for map argument type '%s' must be integer", name.as_string().c_str());
        }
        if (!pair.second->is_singular_scalar()) {
            throw ParseException(pair.first, "key for map argument type '%s' must be single scalar", name.as_string().c_str());
        }
        const auto& value = pair.second->as_scalar()->token();
        if (!value.is_integer()) {
            throw ParseException(pair.first, "key for enum argument type '%s' must be integer", name.as_string().c_str());
        }
        argument_type->entries[pair.first.as_integer()] = value.as_integer();
    }

    return argument_type;
}

std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_range(const Token& name, const ParsedValue *parameters) {
    auto argument_type = std::make_unique<ArgumentTypeRange>();

    if (!parameters->is_scalar()) {
        throw ParseException(parameters->location, "definition of range argument type '%s' must be scalar", name.as_string().c_str());
    }
    auto limits = parameters->as_scalar();
    if (limits->size() == 3) {
        if ((*limits)[0].is_integer() && (*limits)[1] == token_comma && (*limits)[2].is_integer()) {
            // TODO: check for overflow
            argument_type->lower_bound = static_cast<int64_t>((*limits)[0].as_integer());
            argument_type->upper_bound = (*limits)[2].as_integer();
        }
        else {
            throw ParseException(limits->location, "invalid definition of range argument type '%s'", name.as_string().c_str());
        }
    }
    else if (limits->size() == 4) {
        if ((*limits)[0] == token_minus && (*limits)[1].is_integer() && (*limits)[2] == token_comma && (*limits)[3].is_integer()) {
            // TODO: check for overflow
            argument_type->lower_bound = -static_cast<int64_t>((*limits)[1].as_integer());
            argument_type->upper_bound = (*limits)[3].as_integer();
        }
        else {
            throw ParseException(limits->location, "invalid definition of range argument type '%s'", name.as_string().c_str());
        }
    }
    else {
        throw ParseException(limits->location, "invalid definition of range argument type '%s'", name.as_string().c_str());
    }

    return argument_type;
}


AddressingMode::Notation CPUParser::parse_addressing_mode_notation(const AddressingMode& addressing_mode, const ParsedScalar *parameters) {
    AddressingMode::Notation notation;

    for (const auto& token: (*parameters)) {
        if (token.is_name()) {
            auto symbol = argument_symbol(token.as_symbol());
            if (addressing_mode.has_argument(symbol)) {
                notation.elements.emplace_back(AddressingMode::Notation::ARGUMENT, symbol);
            }
            else {
                cpu.add_reserved_word(token.as_symbol());
                notation.elements.emplace_back(AddressingMode::Notation::RESERVED_WORD, token.as_symbol());
            }
        }
        else {
            notation.elements.emplace_back(AddressingMode::Notation::PUNCTUATION, token.as_symbol());
        }
    }

    return notation;
}


symbol_t CPUParser::argument_symbol(symbol_t symbol) {
    auto name = SymbolTable::global[symbol];
    if (name.front() == '.') {
        return symbol;
    }
    return SymbolTable::global.add("$" + name);
}
