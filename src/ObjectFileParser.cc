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
#include "TokenizerSequence.h"

bool ObjectFileParser::initialized = false;
std::map<symbol_t, void (ObjectFileParser::*)()> ObjectFileParser::parser_methods;
Token ObjectFileParser::token_alignment;
Token ObjectFileParser::token_constant;
Token ObjectFileParser::token_data;
Token ObjectFileParser::token_object;
Token ObjectFileParser::token_section;
Token ObjectFileParser::token_size;
Token ObjectFileParser::token_value;

void ObjectFileParser::initialize() {
    if (!initialized) {
        token_alignment = Token(Token::NAME, {}, SymbolTable::global.add("alignment"));
        token_constant = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".constant"));
        token_data = Token(Token::NAME, {}, SymbolTable::global.add("data"));
        token_object = Token(Token::DIRECTIVE, {}, SymbolTable::global.add(".object"));
        token_section = Token(Token::NAME, {}, SymbolTable::global.add("section"));
        token_size = Token(Token::NAME, {}, SymbolTable::global.add("size"));
        token_value = Token(Token::NAME, {}, SymbolTable::global.add("value"));

        parser_methods[token_constant.as_symbol()] = &ObjectFileParser::parse_constant;
        parser_methods[token_object.as_symbol()] = &ObjectFileParser::parse_object;

        initialized = true;
    }
}

ObjectFileParser::ObjectFileParser() {
    initialize();
}

ObjectFile ObjectFileParser::parse(const std::string &filename) {
    if (!parse_file(filename)) {
        throw Exception("can't parse object file '%s'", filename.c_str());
    }
    return file;
}

void ObjectFileParser::parse_directive(const Token &directive) {

}

void ObjectFileParser::parse_constant() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parsed_value = ParsedValue::parse(tokenizer);

    if (!parsed_value->is_dictionary()) {
        throw ParseException(parsed_value->location, "constant is not a dictionary");
    }
    auto parameters = parsed_value->as_dictionary();

    auto value = (*parameters)[token_value];
    if (!value) {
        throw ParseException(parsed_value->location, "constant has no value");
    }
    if (!value->is_scalar()) {
        throw ParseException(parsed_value->location, "constant value is not a scalar");
    }
    // TODO: visibility
    auto value_tokenizer = TokenizerSequence(value->as_scalar()->tokens);
    file.add_constant(name.as_symbol(), Object::NONE, ExpressionParser(value_tokenizer).parse());
}


void ObjectFileParser::parse_object() {
    auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
    auto parsed_value = ParsedValue::parse(tokenizer);

    if (!parsed_value->is_dictionary()) {
        throw ParseException(parsed_value->location, "object is not a dictionary");
    }
    auto parameters = std::dynamic_pointer_cast<ParsedDictionary>(parsed_value);

}


