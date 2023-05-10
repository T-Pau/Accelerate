/*
ObjectFileParser.cc -- 

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

#include "ObjectFileParser.h"
#include "Exception.h"
#include "ParsedValue.h"
#include "ParseException.h"
#include "ExpressionParser.h"
#include "SequenceTokenizer.h"

bool ObjectFileParser::initialized = false;
std::unordered_map<Symbol, void (ObjectFileParser::*)()> ObjectFileParser::parser_methods;
Token ObjectFileParser::token_alignment;
Token ObjectFileParser::token_constant;
Token ObjectFileParser::token_data;
Token ObjectFileParser::token_format_version;
Token ObjectFileParser::token_global;
Token ObjectFileParser::token_local;
Token ObjectFileParser::token_object;
Token ObjectFileParser::token_reserve;
Token ObjectFileParser::token_section;
Token ObjectFileParser::token_target;
Token ObjectFileParser::token_value;
Token ObjectFileParser::token_visibility;

void ObjectFileParser::initialize() {
    if (!initialized) {
        token_alignment = Token(Token::NAME, "alignment");
        token_constant = Token(Token::DIRECTIVE, "constant");
        token_data = Token(Token::NAME, "data");
        token_format_version = Token(Token::DIRECTIVE, "format_version");
        token_global = Token(Token::NAME, "global");
        token_local = Token(Token::NAME, "local");
        token_object = Token(Token::DIRECTIVE, "object");
        token_section = Token(Token::NAME, "section");
        token_reserve = Token(Token::NAME, "size");
        token_target = Token(Token::DIRECTIVE, "target");
        token_value = Token(Token::NAME, "value");
        token_visibility = Token(Token::NAME, "visibility");

        parser_methods[token_format_version.as_symbol()] = &ObjectFileParser::parse_format_version;
        parser_methods[token_constant.as_symbol()] = &ObjectFileParser::parse_constant;
        parser_methods[token_object.as_symbol()] = &ObjectFileParser::parse_object;
        parser_methods[token_target.as_symbol()] = &ObjectFileParser::parse_target;

        initialized = true;
    }
}

ObjectFileParser::ObjectFileParser() {
    initialize();
}

ObjectFile ObjectFileParser::parse(Symbol filename) {
    file = ObjectFile();

    if (!parse_file(filename)) {
        throw Exception("can't parse object file '%s'", filename.c_str());
    }
    return file;
}

void ObjectFileParser::parse_directive(const Token &directive) {
    auto it = parser_methods.find(directive.as_symbol());
    if (it == parser_methods.end()) {
        throw ParseException(directive, "unknown directive");
    }
    (this->*it->second)();
}

void ObjectFileParser::parse_constant() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parse_value = ParsedValue::parse(tokenizer);
    auto parameters = parse_value->as_dictionary();

    auto value = (*parameters)[token_value]->as_scalar();
    auto visibility = visibility_from_name((*parameters)[token_visibility]->as_singular_scalar()->token());
    auto value_tokenizer = SequenceTokenizer(value->tokens);
    file.add_constant(name.as_symbol(), visibility, ExpressionParser(value_tokenizer).parse());
}


void ObjectFileParser::parse_object() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parse_value = ParsedValue::parse(tokenizer);
    auto parameters = parse_value->as_dictionary();

    auto section = (*parameters)[token_section]->as_singular_scalar()->token();
    if (!section.is_name()) {
        throw ParseException(section, "name expected");
    }
    auto visibility = visibility_from_name((*parameters)[token_visibility]->as_singular_scalar()->token());

    auto object = file.create_object(section.as_symbol(), visibility, name);

    auto alignment_value = parameters->get_optional(token_alignment);
    if (alignment_value) {
        auto alignment = alignment_value->as_singular_scalar()->token();
        if (!alignment.is_unsigned()) {
            throw ParseException(alignment, "unsigned integer expected");
        }
        object->alignment = alignment.as_unsigned();
    }

    auto reserve_value = parameters->get_optional(token_reserve);
    if (reserve_value) {
        auto reserve = reserve_value->as_singular_scalar()->token();
        if (!reserve.is_unsigned()) {
            throw ParseException(reserve, "unsigned integer expected");
        }
        object->size = reserve.as_unsigned();
    }

    auto data_value = parameters->get_optional(token_data);
    if (data_value != nullptr) {
        auto tokenizer = SequenceTokenizer(data_value->as_scalar()->tokens);
        object->body.append(ExpressionParser(tokenizer).parse_list());
    }
}

Object::Visibility ObjectFileParser::visibility_from_name(const Token& name) {
    if (name == token_global) {
        return Object::GLOBAL;
    }
    else if (name == token_local) {
        return Object::LOCAL;
    }
    else {
        throw ParseException(name, "illegal visibility");
    }
}

void ObjectFileParser::parse_format_version() {
    // TODO: implement
}

void ObjectFileParser::parse_target() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);

    file.target = &Target::get(name.as_symbol());
}
