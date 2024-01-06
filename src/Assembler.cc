/*
Assembler.cc --

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

#include "Assembler.h"

#include "BodyParser.h"
#include "ExpressionParser.h"
#include "FileReader.h"
#include "LabelBody.h"
#include "ParseException.h"

const Symbol Assembler::symbol_opcode = Symbol(".opcode");
const Token Assembler::token_address = Token{Token::DIRECTIVE, "address"};
const Token Assembler::token_address_name = Token{Token::NAME, "address"};
const Token Assembler::token_align = Token(Token::DIRECTIVE, "align");
const Token Assembler::token_cpu = Token(Token::DIRECTIVE, "cpu");
const Token Assembler::token_data_end = Token(Token::NAME, ".data_end");
const Token Assembler::token_data_size = Token(Token::NAME, ".data_size");
const Token Assembler::token_data_start = Token(Token::NAME, ".data_start");
const Token Assembler::token_default_string_encoding = Token(Token::DIRECTIVE, "default_string_encoding");
const Token Assembler::token_extension = Token(Token::DIRECTIVE, "extension");
const Token Assembler::token_macro = Token(Token::DIRECTIVE, "macro");
const Token Assembler::token_output = Token(Token::DIRECTIVE, "output");
const Token Assembler::token_pin = Token(Token::DIRECTIVE, "pin");
const Token Assembler::token_read_only = Token(Token::NAME, "read_only");
const Token Assembler::token_read_write = Token(Token::NAME, "read_write");
const Token Assembler::token_reserve = Token(Token::DIRECTIVE, "reserve");
const Token Assembler::token_reserve_only = Token(Token::NAME, "reserve_only");
const Token Assembler::token_section = Token(Token::DIRECTIVE, "section");
const Token Assembler::token_segment = Token(Token::DIRECTIVE, "segment");
const Token Assembler::token_segment_name = Token(Token::NAME, "segment");
const Token Assembler::token_string_encoding = Token(Token::DIRECTIVE, "string_encoding");
const Token Assembler::token_target = Token(Token::DIRECTIVE, "target");
const Token Assembler::token_use = Token(Token::DIRECTIVE, "use");
const Token Assembler::token_used = Token(Token::DIRECTIVE, "used");
const Token Assembler::token_uses = Token(Token::DIRECTIVE, "uses");
const Token Assembler::token_type = Token(Token::NAME, "type");
const Token Assembler::token_visibility = Token(Token::DIRECTIVE, "visibility");

// clang-format off
const std::unordered_map<Token, Assembler::Directive> Assembler::directives = {
    {token_cpu, Directive{&Assembler::parse_cpu, true}},
    {token_default_string_encoding, Directive{&Assembler::parse_default_string_encoding, true}},
    {token_extension, Directive{&Assembler::parse_extension, true}},
    {token_output, Directive{&Assembler::parse_output, true}},
    {token_pin, Directive{&Assembler::parse_pin}},
    {token_section, Directive{&Assembler::parse_section}},
    {token_segment, Directive{&Assembler::parse_segment, true}},
    {token_string_encoding, Directive{&Assembler::parse_string_encoding}},
    {token_target, Directive{&Assembler::parse_target}},
    {token_use, Directive{&Assembler::parse_use}},
    {token_visibility, Directive{&Assembler::parse_visibility}}
};
// clang-format on

Assembler::Assembler(const Target* target, const Path& path, const std::unordered_set<Symbol>& defines) : tokenizer{path, true, defines} { set_target(target); }

Target Assembler::parse_target(Symbol name, Symbol file_name) {
    parsing_target = true;
    parsed_target.name = name;
    target = &parsed_target;

    ParsedValue::setup(tokenizer);
    tokenizer.add_literal(token_data_end);
    tokenizer.add_literal(token_data_size);
    tokenizer.add_literal(token_data_start);

    object_file = target->object_file;
    parse(file_name);
    parsed_target.defines = std::move(tokenizer.defines);
    return std::move(parsed_target);
}

std::shared_ptr<ObjectFile> Assembler::parse_object_file(Symbol file_name) {
    parsing_target = false;
    object_file = std::make_shared<ObjectFile>();
    parse(file_name);
    return object_file;
}

void Assembler::parse(Symbol file_name) {
    if (target) {
        Target::set_current_target(target);
        target->cpu->setup(tokenizer);
        tokenizer.define(target->defines);
    }
    ExpressionParser::setup(tokenizer);
    BodyParser::setup(tokenizer);
    tokenizer.add_punctuations({"{", "}", "=", ":"});
    tokenizer.push(file_name);

    object_file->target = target;

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    break;

                case Token::DIRECTIVE:
                    parse_directive(token);
                    break;

                case Token::NAME:
                    parse_name(current_visibility, token);
                    break;

                case Token::NEWLINE:
                    // ignore
                    break;

                case Token::INSTRUCTION:
                    throw ParseException(token, "instruction not allowed outside symbol");

                case Token::PUNCTUATION:
                case Token::VALUE:
                case Token::PREPROCESSOR:
                case Token::STRING:
                case Token::KEYWORD:
                    throw ParseException(token, "unexpected %s", token.type_name());
            }
        } catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
            tokenizer.skip_until(TokenGroup::newline, true);
        }
    }

    object_file->evaluate();

    Target::clear_current_target();
}

void Assembler::parse_assignment(Visibility visibility, const Token& name) {
    auto value = ExpressionParser(tokenizer).parse();
    object_file->add_constant(std::make_unique<ObjectFile::Constant>(object_file.get(), name, visibility, value));
}

void Assembler::parse_cpu() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);

    try {
        had_cpu = true;
        parsed_target.cpu = &CPU::get(name.as_symbol(), tokenizer.current_file());
    } catch (Exception& ex) {
        throw ParseException(name, "%s", ex.what());
    }
}

void Assembler::parse_default_string_encoding() {
    auto token = tokenizer.expect(Token::NAME, TokenGroup::newline);
    parsed_target.default_string_encoding = parsed_target.string_encoding(token.as_symbol());
    if (!parsed_target.default_string_encoding) {
        throw ParseException(token, "unknown string encoding '%s'", token.as_string().c_str());
    }
}

void Assembler::parse_pin() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto address = ExpressionParser(tokenizer).parse();
    object_file->pin(name.as_symbol(), address);
}

void Assembler::parse_section() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);

    if (tokenizer.peek().is_end_of_line()) {
        if (!target) {
            throw ParseException(name, "no target specified");
        }
        if (!target->map.has_section(name.as_symbol())) {
            throw ParseException(name, "unknown section");
        }
        current_section = name.as_symbol();
    }
    else if (!parsing_target) {
        throw ParseException(tokenizer.current_location(), "expected newline");
    }
    else {
        auto parse_value = ParsedValue::parse(tokenizer);
        tokenizer.unget(Token{Token::NEWLINE, tokenizer.current_location()});
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
            if (parameters->has_key(token_address_name)) {
                throw ParseException(name, "segment and address are mutually exclusive");
            }

            auto segment_blocks = parsed_target.map.segment(segment->as_singular_scalar()->token().as_symbol());
            if (segment_blocks == nullptr) {
                throw ParseException(segment->as_singular_scalar()->token(), "unknown segment");
            }
            blocks.insert(blocks.begin(), segment_blocks->begin(), segment_blocks->end());
        }
        else {
            blocks = parse_address((*parameters)[token_address_name].get());
        }

        parsed_target.map.add_section(MemoryMap::Section(name.as_symbol(), type, std::move(blocks)));
    }
}

void Assembler::parse_segment() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parse_value = ParsedValue::parse(tokenizer);
    tokenizer.unget(Token{Token::NEWLINE, tokenizer.current_location()});
    auto parameters = parse_value->as_dictionary();

    if (segment_names.contains(name)) {
        throw ParseException(name, "duplicate segment definition"); // TODO: attach note
    }
    segment_names.insert(name);

    parsed_target.map.add_segment(name.as_symbol(), parse_address((*parameters)[token_address_name].get()));
}

void Assembler::parse_symbol(Visibility visibility, const Token& name) {
    if (!target) {
        // TODO: skip until matching curly close
        tokenizer.skip_until(Token::curly_close, true);
        throw ParseException(name, "no target specified");
    }
    auto object = object_file->create_object(current_section, visibility, name);

    while (true) {
        auto token = tokenizer.next();
        if (token.is_newline()) {
            tokenizer.unget(token);
            break;
        }
        else if (token == Token::curly_open) {
            // TODO: error if .reserved
            object->body = BodyParser(tokenizer, object, target->cpu, &tokenizer.defines).parse();
            break;
        }
        // TODO: parameters
        else if (token == token_address) {
            object->address = Address(tokenizer, object_file->private_environment);
        }
        else if (token == token_align || token == token_reserve) {
            auto expression = ExpressionParser(tokenizer).parse();

            if (token == token_align) {
                auto value = expression.value();

                if (!value.has_value() || !value->is_unsigned()) {
                    throw ParseException(expression.location(), "alignment must be constant unsigned integer");
                }
                object->alignment = value->unsigned_value();
            }
            else {
                object->reservation_expression = expression;
            }
        }
        else if (token == token_uses) {
            auto object_name = tokenizer.expect(Token::NAME);
            object->uses(object_name.as_symbol());
        }
        else if (token == token_used) {
            object_file->mark_used(object);
        }
        else {
            throw ParseException(token, "unexpected");
        }
    }

    if (current_section.empty()) {
        throw ParseException(name, "symbol outside section");
    }
    if (object->empty()) {
        throw ParseException(name, "empty symbol");
    }

    // TODO: warn if reserved in saved section
    // TODO: error if data in unsaved section
}

void Assembler::parse_directive(const Token& directive) {
    if (auto visibility = VisibilityHelper::from_token(directive)) {
        auto name = tokenizer.next();
        if (name == token_macro) {
            parse_macro(*visibility);
        }
        else if (name.get_type() != Token::NAME) {
            throw ParseException(name, "name expected");
        }
        else {
            parse_name(*visibility, name);
        }
    }
    else if (directive == token_macro) {
        parse_macro(current_visibility);
    }
    else {
        auto it = directives.find(directive);
        if (it != directives.end() && (parsing_target || !it->second.target_only)) {
            (this->*it->second.parse)();
        }
        else {
            throw ParseException(directive, "unknown directive");
        }
    }
    tokenizer.expect(Token::NEWLINE, TokenGroup::newline);
}

void Assembler::parse_extension() {
    auto token = tokenizer.expect(Token::STRING, TokenGroup::newline);

    parsed_target.extension = token.as_string();
}

void Assembler::parse_output() {
    auto token = tokenizer.next();
    if (token != Token::curly_open) {
        tokenizer.skip_until(TokenGroup::newline, true);
        throw ParseException(token, "expected '{'");
    }

    parsed_target.output = BodyParser(tokenizer, parsed_target.cpu).parse();
}

void Assembler::parse_string_encoding() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    if (tokenizer.peek().is_end_of_line()) {
        // TODO: set current string encoding
    }
    else if (!parsing_target) {
        throw ParseException(tokenizer.current_location(), "expected newline");
    }
    else {
        auto parse_value = ParsedValue::parse(tokenizer);
        tokenizer.unget(Token{Token::NEWLINE, tokenizer.current_location()});
        if (parsed_target.string_encoding(name.as_symbol())) {
            throw ParseException(name, "duplicate string encoding '%s'", name.as_string().c_str());
        }
        parsed_target.string_encodings[name.as_symbol()] = StringEncoding(name.as_symbol(), parse_value, parsed_target);
    }
}

void Assembler::parse_target() {
    if (parsing_target) {
        throw ParseException(tokenizer.current_location(), "unknown directive");
    }

    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);
    // TODO: check that target comes first
    auto& new_target = Target::get(name.as_symbol());
    if (target) {
        // TODO: check that targets are compatible
    }
    else {
        target = &new_target;
        object_file->target = target;
    }
}

void Assembler::parse_use() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);

    object_file->mark_used(name.as_symbol());
}

void Assembler::parse_visibility() {
    auto name = tokenizer.expect(Token::NAME);
    if (auto visibility = VisibilityHelper::from_token(name)) {
        current_visibility = *visibility;
    }
    else {
        throw ParseException(name, "unknown visibility");
    }
}

void Assembler::set_target(const Target* new_target) {
    if (target) {
        // TODO: check that target and new_target are compatible
    }
    target = new_target;
    if (target) {
        tokenizer.define(target->defines);
    }
}

void Assembler::parse_name(Visibility visibility, const Token& name) {
    auto token = tokenizer.next();
    if (token == Token::equals) {
        parse_assignment(visibility, name);
        return;
    }
    else if (token == Token::paren_open) {
        auto arguments = Callable::Arguments(tokenizer);
        tokenizer.expect(Token::paren_close);
        tokenizer.expect(Token::equals);

        auto definition = ExpressionParser(tokenizer).parse();
        object_file->add_function(std::make_unique<Function>(object_file.get(), name, visibility, arguments, definition));
    }
    else {
        tokenizer.unget(token);
        parse_symbol(visibility, name);
    }
}

void Assembler::parse_macro(Visibility visibility) {
    auto name = tokenizer.expect(Token::NAME);
    auto arguments = Callable::Arguments(tokenizer);
    tokenizer.expect(Token::curly_open);
    auto body = BodyParser(tokenizer, object_file->target->cpu).parse(); // TODO: proper mode for macros
    object_file->add_macro(std::make_unique<Macro>(object_file.get(), name, visibility, arguments, body));
}

std::vector<MemoryMap::Block> Assembler::parse_address(const ParsedValue* address) {
    auto blocks = std::vector<MemoryMap::Block>();

    if (address->is_array()) {
        for (const auto& element : *address->as_array()) {
            blocks.emplace_back(parse_single_address(element->as_scalar()));
        }
    }
    else {
        blocks.emplace_back(parse_single_address(address->as_scalar()));
    }

    return blocks;
}

MemoryMap::AccessType Assembler::parse_type(const Token& type) {
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

MemoryMap::Block Assembler::parse_single_address(const ParsedScalar* address) {
    size_t index = 0;
    uint64_t bank = 0;

    if (address->size() > 2 && (*address)[1] == Token::colon) {
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
    uint64_t start = parse_address_part(start_token);
    uint64_t size = 1;

    if (address->size() == index + 3 && (*address)[index + 1] == Token::minus) {
        auto end_token = (*address)[index + 2];
        size = parse_address_part(end_token) - start + 1;
    }
    else if (address->size() != index + 1) {
        throw ParseException(address->location, "invalid address specification");
    }

    return {bank, start, size};
}

uint64_t Assembler::parse_address_part(const Token& token) {
    if (token.is_name()) {
        auto constant = object_file->constant(token.as_symbol());
        if (!constant->value.has_value()) {
            throw ParseException(token, "unresolved constant");
        }
        return constant->value.value()->unsigned_value();
    }
    else if (token.is_unsigned()) {
        return token.as_unsigned();
    }
    else {
        throw ParseException(token, "unsigned integer or constant expected");
    }
}