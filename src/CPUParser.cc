/*
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

#include <tpau-cpp-kernal/LocationException.h>

#include "AddressingMode.h"
#include "Body/DataBody.h"
#include "CPUGetter.h"
#include "Expression/ObjectNameExpression.h"
#include "Expression/ValueExpression.h"
#include "ExpressionParser.h"
#include "LibraryParser.h"
#include "SequenceTokenizer.h"
#include "StructuredArray.h"
#include "StructuredDictionary.h"
#include "StructuredScalar.h"
#include "StructuredValue.h"

using namespace tpau::cpp_kernal;

std::unordered_map<Symbol, std::unique_ptr<ArgumentType> (CPUParser::*)(const Token& name, const StructuredValue* parameters)> CPUParser::argument_type_parser_methods;
std::unordered_map<Symbol, void (CPUParser::*)()> CPUParser::parser_methods;

bool CPUParser::initialized = false;
Token CPUParser::token_arguments;
Token CPUParser::token_any;
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
        token_arguments = Token(Token::NAME, "arguments");
        token_any = Token(Token::NAME, "any");
        token_comma = Token(Token::PUNCTUATION, ",");
        token_encoding = Token(Token::NAME, "encoding");
        token_keywords = Token(Token::NAME, "keywords");
        token_minus = Token(Token::PUNCTUATION, "-");
        token_notation = Token(Token::NAME, "notation");
        token_opcode = Token(Token::NAME, ".opcode");
        token_pc = Token(Token::NAME, ".pc");
        token_punctuation = Token(Token::NAME, "punctuation");

        parser_methods[Symbol("byte_order")] = &CPUParser::parse_byte_order;
        parser_methods[Symbol("addressing_mode")] = &CPUParser::parse_addressing_mode;
        parser_methods[Symbol("argument_type")] = &CPUParser::parse_argument_type;
        parser_methods[Symbol("instruction")] = &CPUParser::parse_instruction;
        parser_methods[Symbol("syntax")] = &CPUParser::parse_syntax;

        argument_type_parser_methods[Symbol("enum")] = &CPUParser::parse_argument_type_enum;
        argument_type_parser_methods[Symbol("fits")] = &CPUParser::parse_argument_type_encoding;
        argument_type_parser_methods[Symbol("map")] = &CPUParser::parse_argument_type_map;
        argument_type_parser_methods[Symbol("range")] = &CPUParser::parse_argument_type_range;

        initialized = true;
    }
}

CPUParser::CPUParser() : FileParser(*CPUGetter::global.search_path) {
    initialize();

    tokenizer.add_punctuations({"-", ",", "="});
    tokenizer.add_literal(token_opcode);
    tokenizer.add_literal(token_pc);
}

CPU CPUParser::parse(Symbol file_name) {
    cpu = CPU();

    if (!parse_file(file_name)) {
        throw Exception("can't parse CPU file '{}'", file_name);
    }
    return std::move(cpu);
}

void CPUParser::parse_directive(const Token& directive) {
    auto it = parser_methods.find(directive.as_symbol());
    if (it == parser_methods.end()) {
        throw LocationException(directive.location, "unknown directive");
    }
    (this->*it->second)();
}

void CPUParser::parse_addressing_mode() {
    Token name = tokenizer.expect(Token::NAME, group_directive);
    auto parameters = StructuredValue::parse(tokenizer);

    {
        auto it = addressing_mode_names.find(name);
        if (it != addressing_mode_names.end()) {
            throw LocationException(name.location, "duplicate definition of addressing mode '{}'", name);
            // TODO: attach note it->second,  previously defined here
        }
        addressing_mode_names.insert(name);
    }

    if (!parameters->is_dictionary()) {
        throw LocationException(name.location, "addressing mode definition is not a dictionary");
    }

    auto unencoded_encoding_arguments = std::unordered_set<Symbol>();
    auto unused_arguments = std::unordered_set<Symbol>();

    auto addressing_mode = AddressingMode();

    auto definition = parameters->as_dictionary();

    auto arguments = definition->get_optional(token_arguments);
    if (arguments != nullptr) {
        if (!arguments->is_dictionary()) {
            throw LocationException(name.location, "addressing mode definition is not a dictionary");
        }
        for (const auto& pair : (*arguments->as_dictionary())) {
            if (!pair.first.is_name()) {
                throw LocationException(pair.first.location, "expected {}, got {}", Token::type_name(Token::NAME), pair.first.type_name());
            }
            if (!pair.second->is_scalar()) {
                throw LocationException(pair.second->location, "invalid argument type");
            }
            auto argument_definition = pair.second->as_scalar();
            if (argument_definition->size() != 1 && (argument_definition->size() != 3 && (*argument_definition)[1] != Token::equals)) {
                throw LocationException(pair.second->location, "invalid argument type");
            }
            auto argument_type_name = pair.second->as_scalar()->token();
            if (!argument_type_name.is_name()) {
                throw LocationException(argument_type_name.location, "expected {}, got {}", Token::type_name(Token::NAME), argument_type_name.type_name());
            }
            auto argument_type = cpu.argument_type(argument_type_name.as_symbol());
            if (argument_type == nullptr) {
                throw LocationException(argument_type_name.location, "unknown argument type '{}'", argument_type_name);
            }
            auto default_value = std::optional<Value>{};
            if (argument_definition->size() > 1) {
                auto& default_value_token = (*argument_definition)[2];
                if (!default_value_token.is_integer()) {
                    throw LocationException(default_value_token.location, "default value must be integer");
                }
                default_value = Value{default_value_token.as_value()};
            }
            addressing_mode.arguments[pair.first.as_symbol()] = std::make_unique<AddressingMode::Argument>(argument_type, default_value);
            if (argument_type->is<ArgumentTypeEncoding>()) {
                auto argument_variable_name = pair.first.as_symbol();
                unencoded_encoding_arguments.insert(argument_variable_name);
            }
            unused_arguments.insert(pair.first.as_symbol());
        }
    }

    auto notation = (*definition)[token_notation];
    if (notation == nullptr) {
        throw LocationException(name.location, "notation missing for addressing mode '{}'", name);
    }
    else if (notation->is_scalar()) {
        addressing_mode.add_notation(parse_addressing_mode_notation(addressing_mode, notation->as_scalar()));
    }
    else if (notation->is_array()) {
        for (const auto& n : (*notation->as_array())) {
            if (!n->is_scalar()) {
                throw LocationException(notation->location, "invalid notation for addressing mode '{}'", name);
            }
            addressing_mode.add_notation(parse_addressing_mode_notation(addressing_mode, n->as_scalar()));
        }
    }
    else {
        throw LocationException(notation->location, "invalid notation for addressing mode '{}'", name);
    }

    auto encoding_definition = definition->get_optional(token_encoding);
    if (encoding_definition == nullptr) {
        auto expression = Expression{};
        if (token_opcode == LibraryParser::token_object_name) {
            expression = ObjectNameExpression::create(token_opcode.location, {});
        }
        else if (token_opcode.is_name() || token_opcode == Token::colon_minus || token_opcode == Token::colon_plus) {
            expression = NameExpression::create(token_opcode.location, token_opcode.as_symbol());
        }
        else {
            expression = ValueExpression::create(token_opcode.location, token_opcode.as_value());
        }

        addressing_mode.encoding = DataBody::create({{expression, {}}});
    }
    else if (encoding_definition->is_scalar()) {
        auto encoding_tokens = std::vector<Token>();

        for (auto const& token : encoding_definition->as_scalar()->tokens) {
            if (token == token_pc) {
                addressing_mode.uses_pc = true;
            }
            if (token.is_name() && !(token == token_opcode || token == token_pc)) {
                auto argument_name = token.as_symbol();
                if (!addressing_mode.has_argument(argument_name)) {
                    throw LocationException(token.location, "unknown argument in encoding");
                }
                unused_arguments.erase(token.as_symbol());
                encoding_tokens.emplace_back(Token::NAME, token.location, argument_name);
            }
            else {
                encoding_tokens.emplace_back(token);
            }
        }

        auto encoding_tokenizer = SequenceTokenizer(encoding_tokens);
        addressing_mode.encoding = ExpressionParser(encoding_tokenizer).parse_list();
        for (auto& datum : addressing_mode.encoding.as<DataBody>()->data) {
            if (datum.expression.is<NameExpression>() && datum.expression.as<NameExpression>()->variable() != token_opcode.as_symbol()) {
                auto variable_name = datum.expression.as<NameExpression>()->variable();
                auto encoding_type = addressing_mode.argument(variable_name)->type->as<ArgumentTypeEncoding>();
                if (encoding_type && (!datum.encoding || *datum.encoding == encoding_type->encoding)) {
                    datum.encoding = Encoder{encoding_type->encoding};
                    unencoded_encoding_arguments.erase(variable_name);
                }
            }
        }
    }
    else {
        throw LocationException(name.location, "invalid encoding for addressing mode '{}'", name);
    }

    // TODO: warn unused arguments

    for (const auto& argument_name : unencoded_encoding_arguments) {
        auto argument = addressing_mode.argument(argument_name);
        if (!argument) {
            throw Exception("internal error: expected encoding argument type for '{}'", argument_name);
        }
        auto encoding_type = argument->type;
        if (!encoding_type) {
            throw Exception("internal error: expected encoding argument type for '{}'", argument_name);
        }
        addressing_mode.arguments[argument_name] = std::make_unique<AddressingMode::Argument>(cpu.argument_type(Symbol(".range(" + encoding_type->name.str() + ")")));
    }

    cpu.add_addressing_mode(name.as_symbol(), std::move(addressing_mode));
}

void CPUParser::parse_argument_type() {
    Token name = tokenizer.expect(Token::NAME, group_directive);
    Token type = tokenizer.expect(Token::NAME, group_directive);

    auto argument_type = std::unique_ptr<ArgumentType>();

    if (type == token_any) {
        argument_type = std::make_unique<ArgumentTypeAny>(name.as_symbol());
    }
    else {
        auto parameters = StructuredValue::parse(tokenizer);

        {
            auto it = argument_type_names.find(name);
            if (it != argument_type_names.end()) {
                throw LocationException(type.location, "duplicate definition of argument type '{}'", name);
                // TODO: attach note it->second,  previously defined here
            }
            argument_type_names.insert(name);
        }

        auto it = argument_type_parser_methods.find(type.as_symbol());
        if (it == argument_type_parser_methods.end()) {
            throw LocationException(type.location, "unknown argument type '{}'", type);
        }
        argument_type = (this->*it->second)(name, parameters.get());
        if (auto encoding_type = argument_type->as<ArgumentTypeEncoding>()) {
            auto range_name = Symbol(".range(" + name.as_string() + ")");
            cpu.add_argument_type(range_name, encoding_type->range_type(range_name));
        }
    }

    // TODO: add .range(name) for encoding type
    cpu.add_argument_type(name.as_symbol(), std::move(argument_type));
}

void CPUParser::parse_byte_order() {
    Token byte_order = tokenizer.expect(Token::VALUE, group_directive);
    tokenizer.skip(Token::NEWLINE);

    cpu.byte_order = byte_order.as_unsigned();
}

void CPUParser::parse_instruction() {
    Token name = tokenizer.next();

    if (name == Token::curly_open) {
        tokenizer.unget(name);
        name = Token(Token::NAME, name.location, Symbol());
    }
    else if (name.get_type() != Token::NAME) {
        throw LocationException(name.location, "expected name or '{'");
    }
    auto parameters = StructuredValue::parse(tokenizer);

    if (!parameters->is_dictionary()) {
        throw LocationException(parameters->location, "instruction definition must be dictionary");
    }

    auto& instruction = cpu.instructions[name.as_symbol()];

    for (const auto& pair : (*parameters->as_dictionary())) {
        if (!pair.first.is_name()) {
            throw LocationException(pair.first.location, "addressing mode must be name");
        }
        if (!addressing_mode_names.contains(pair.first)) {
            throw LocationException(pair.first.location, "unknown addressing mode");
        }
        auto it = instruction.opcodes.find(pair.first.as_symbol());
        if (it != instruction.opcodes.end()) {
            throw LocationException(pair.first.location, "redefinition of addressing mode");
        }
        if (!pair.second->is_singular_scalar() || !pair.second->as_scalar()->token().is_unsigned()) {
            throw LocationException(pair.second->location, "opcode must be unsigned integer");
        }
        instruction.opcodes[pair.first.as_symbol()] = pair.second->as_scalar()->token().as_unsigned();
    }
}

void CPUParser::parse_syntax() {
    auto type = tokenizer.expect(Token::NAME, group_directive);
    auto values = StructuredValue::parse(tokenizer);

    if (!values->is_scalar()) {
        throw LocationException(values->location, "expected strings");
    }

    if (type == token_keywords) {
        for (const auto& value : (*values->as_scalar())) {
            if (!value.is_string()) {
                throw LocationException(value.location, "expected string");
            }
            cpu.add_reserved_word(value.as_symbol());
            tokenizer.add_literal(Token::NAME, value.as_string());
        }
    }
    else if (type == token_punctuation) {
        for (const auto& value : (*values->as_scalar())) {
            if (!value.is_string()) {
                throw LocationException(value.location, "expected string");
            }
            cpu.add_punctuation(value.as_symbol());
            tokenizer.add_literal(Token::PUNCTUATION, value.as_string());
        }
    }
}

std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_encoding(const Token& name, const StructuredValue* parameters) {
    if (!parameters->is_scalar()) {
        throw LocationException(parameters->location, "definition of range argument type '{}' must be scalar", name);
    }
    auto tokenizer = SequenceTokenizer(parameters->as_scalar()->tokens);
    tokenizer.unget(Token::colon);
    auto encoding = ExpressionParser(tokenizer).parse_encoding();
    if (!encoding || !encoding->is_integer_encoder()) {
        throw LocationException(parameters->location, "invalid encoding");
    }
    return std::make_unique<ArgumentTypeEncoding>(name.as_symbol(), *encoding->as_integer_encoder());
}

std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_enum(const Token& name, const StructuredValue* parameters) {
    auto argument_type = std::make_unique<ArgumentTypeEnum>(name.as_symbol());

    if (!parameters->is_dictionary()) {
        throw LocationException(parameters->location, "definition of enum argument type '{}' must be dictionary", name);
    }

    for (const auto& pair : (*parameters->as_dictionary())) {
        if (!pair.first.is_name()) {
            throw LocationException(pair.first.location, "key for enum argument type '{}' must be name", name);
        }
        if (!pair.second->is_singular_scalar()) {
            throw LocationException(pair.first.location, "key for enum argument type '{}' must be single scalar", name);
        }
        const auto& value = pair.second->as_scalar()->token();
        if (!value.is_integer()) {
            throw LocationException(pair.first.location, "key for enum argument type '{}' must be integer", name);
        }
        auto symbol = pair.first.as_symbol();
        cpu.add_reserved_word(symbol);
        argument_type->entries[symbol] = value.as_value();
    }

    return argument_type;
}

std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_map(const Token& name, const StructuredValue* parameters) {
    auto argument_type = std::make_unique<ArgumentTypeMap>(name.as_symbol());

    if (!parameters->is_dictionary()) {
        throw LocationException(parameters->location, "definition of map argument type '{}' must be dictionary", name);
    }

    for (const auto& pair : (*parameters->as_dictionary())) {
        if (!pair.first.is_integer()) {
            throw LocationException(pair.first.location, "key for map argument type '{}' must be integer", name);
        }
        if (!pair.second->is_singular_scalar()) {
            throw LocationException(pair.first.location, "key for map argument type '{}' must be single scalar", name);
        }
        const auto& value = pair.second->as_scalar()->token();
        if (!value.is_integer()) {
            throw LocationException(pair.first.location, "key for map argument type '{}' must be integer", name);
        }
        argument_type->entries[pair.first.as_value()] = value.as_value();
    }

    return argument_type;
}

std::unique_ptr<ArgumentType> CPUParser::parse_argument_type_range(const Token& name, const StructuredValue* parameters) {
    auto argument_type = std::make_unique<ArgumentTypeRange>(name.as_symbol());

    if (!parameters->is_scalar()) {
        throw LocationException(parameters->location, "definition of range argument type '{}' must be scalar", name);
    }
    auto limits = parameters->as_scalar();
    if (limits->size() == 3) {
        if ((*limits)[0].is_integer() && (*limits)[1] == token_comma && (*limits)[2].is_integer()) {
            argument_type->lower_bound = (*limits)[0].as_value();
            argument_type->upper_bound = (*limits)[2].as_value();
        }
        else {
            throw LocationException(limits->location, "invalid definition of range argument type '{}'", name);
        }
    }
    else if (limits->size() == 4) {
        if ((*limits)[0] == token_minus && (*limits)[1].is_integer() && (*limits)[2] == token_comma && (*limits)[3].is_integer()) {
            argument_type->lower_bound = -(*limits)[1].as_value();
            argument_type->upper_bound = (*limits)[3].as_value();
        }
        else {
            throw LocationException(limits->location, "invalid definition of range argument type '{}'", name);
        }
    }
    else {
        throw LocationException(limits->location, "invalid definition of range argument type '{}'", name);
    }

    return argument_type;
}

AddressingMode::Notation CPUParser::parse_addressing_mode_notation(const AddressingMode& addressing_mode, const StructuredScalar* parameters) {
    AddressingMode::Notation notation;

    for (const auto& token : (*parameters)) {
        if (token.is_name()) {
            auto symbol = token.as_symbol();
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
