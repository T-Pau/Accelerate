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
#include "LibraryGetter.h"
#include "ParsedValue.h"
#include "ParseException.h"
#include "ExpressionParser.h"
#include "SequenceTokenizer.h"

bool ObjectFileParser::initialized = false;
std::unordered_map<Symbol, void (ObjectFileParser::*)()> ObjectFileParser::parser_methods;
std::unordered_map<Symbol, void (ObjectFileParser::*)(Token name, const std::shared_ptr<ParsedValue>& definition)> ObjectFileParser::symbol_parser_methods;
Token ObjectFileParser::token_constant;
Token ObjectFileParser::token_format_version;
Token ObjectFileParser::token_function;
Token ObjectFileParser::token_import;
Token ObjectFileParser::token_in_range;
Token ObjectFileParser::token_label_offset;
Token ObjectFileParser::token_macro;
Token ObjectFileParser::token_object;
Token ObjectFileParser::token_object_name;
Token ObjectFileParser::token_target;

void ObjectFileParser::initialize() {
    if (!initialized) {
        token_constant = Token(Token::DIRECTIVE, "constant");
        token_format_version = Token(Token::DIRECTIVE, "format_version");
        token_function = Token(Token::DIRECTIVE, "function");
        token_import = Token(Token::DIRECTIVE, "import");
        token_in_range = Token(Token::NAME, ".in_range");
        token_label_offset = Token(Token::NAME, ".label_offset");
        token_macro = Token(Token::DIRECTIVE, "macro");
        token_object = Token(Token::DIRECTIVE, "object");
        token_object_name = Token(Token::NAME, ".current_object");
        token_target = Token(Token::DIRECTIVE, "target");

        parser_methods[token_format_version.as_symbol()] = &ObjectFileParser::parse_format_version;
        parser_methods[token_target.as_symbol()] = &ObjectFileParser::parse_target;
        parser_methods[token_import.as_symbol()] = &ObjectFileParser::parse_import;
        symbol_parser_methods[token_constant.as_symbol()] = &ObjectFileParser::parse_constant;
        symbol_parser_methods[token_function.as_symbol()] = &ObjectFileParser::parse_function;
        symbol_parser_methods[token_object.as_symbol()] = &ObjectFileParser::parse_object;
        symbol_parser_methods[token_macro.as_symbol()] = &ObjectFileParser::parse_macro;

        initialized = true;
    }
}

ObjectFileParser::ObjectFileParser() {
    initialize();
    tokenizer.add_literal(token_in_range);
    tokenizer.add_literal(token_label_offset);
    tokenizer.add_literal(token_object);
    tokenizer.add_literal(token_object_name);
    tokenizer.add_literal(Token::NAME, ".label_", "0123456789");
    tokenizer.add_literal(Token::colon_minus);
    tokenizer.add_literal(Token::colon_plus);
}

std::shared_ptr<ObjectFile> ObjectFileParser::parse(Symbol filename) {
    file = std::make_shared<ObjectFile>();
    file->name = filename;

    if (!parse_file(filename)) {
        throw Exception("can't parse object file '%s'", filename.c_str());
    }
    return file;
}

void ObjectFileParser::parse_directive(const Token &directive) {
    auto it = parser_methods.find(directive.as_symbol());
    if (it != parser_methods.end()) {
        (this->*it->second)();
    }
    else {
        auto it_symbol = symbol_parser_methods.find(directive.as_symbol());
        if (it_symbol != symbol_parser_methods.end()) {
            (this->*it_symbol->second)(tokenizer.expect(Token::NAME, TokenGroup::newline), ParsedValue::parse(tokenizer));
        }
        else {
            throw ParseException(directive, "unknown directive");
        }
    }
}


void ObjectFileParser::parse_constant(Token name, const std::shared_ptr<ParsedValue>& definition) {
    file->add_constant(std::make_unique<ObjectFile::Constant>(name, definition));
}


void ObjectFileParser::parse_object(Token name, const std::shared_ptr<ParsedValue>& definition) {
    file->add_object(std::make_unique<Object>(file.get(), name, definition));
}


void ObjectFileParser::parse_format_version() {
    auto token = tokenizer.expect(Token::VALUE);
    // TODO: implement
}

void ObjectFileParser::parse_function(Token name, const std::shared_ptr<ParsedValue>& definition) {
   file->add_function(std::make_unique<Function>(name, definition));
}

void ObjectFileParser::parse_target() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);

    file->set_target(&Target::get(name.as_symbol()));
}

void ObjectFileParser::parse_macro(Token name, const std::shared_ptr<ParsedValue>& definition) {
    file->add_macro(std::make_unique<Macro>(name, definition));
}

void ObjectFileParser::parse_import() {
    auto first = true;

    while (true) {
        auto token = tokenizer.next();
        if (!token || token.is_newline()) {
            break;
        }
        if (first) {
            first = false;
        }
        else {
            if (token != Token::comma) {
                throw ParseException(token, "expected ','");
            }
            token = tokenizer.next();
        }
        if (!token.is_string()) {
            throw ParseException(token, "expected string");
        }
        file->import(LibraryGetter::global.get(token.as_symbol(), file->name).get());
    }
}
