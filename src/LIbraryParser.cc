/*
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

#include "LibraryParser.h"

#include <tpau-cpp-kernal/LocationException.h>

#include "Entity/Object.h"
#include "ExpressionParser.h"
#include "LibraryGetter.h"
#include "StructuredValue.h"
#include "Target.h"

using namespace tpau::cpp_kernal;

const Token LibraryParser::token_constant = Token(Token::DIRECTIVE, "constant");
const Token LibraryParser::token_format_version = Token(Token::DIRECTIVE, "format_version");
const Token LibraryParser::token_function = Token(Token::DIRECTIVE, "function");
const Token LibraryParser::token_import = Token(Token::DIRECTIVE, "import");
const Token LibraryParser::token_in_range = Token(Token::NAME, ".in_range");
const Token LibraryParser::token_label_offset = Token(Token::NAME, ".label_offset");
const Token LibraryParser::token_macro = Token(Token::DIRECTIVE, "macro");
const Token LibraryParser::token_object = Token(Token::DIRECTIVE, "object");
const Token LibraryParser::token_object_name = Token(Token::NAME, ".current_object");
const Token LibraryParser::token_pin = Token(Token::DIRECTIVE, "pin");
const Token LibraryParser::token_target = Token(Token::DIRECTIVE, "target");
const Token LibraryParser::token_use = Token(Token::DIRECTIVE, "use");

// clang-format off
const std::unordered_map<Symbol, void (LibraryParser::*)()> LibraryParser::parser_methods ={
    {token_format_version.as_symbol(), &LibraryParser::parse_format_version},
    {token_import.as_symbol(), &LibraryParser::parse_import},
    {token_pin.as_symbol(), &LibraryParser::parse_pin},
    {token_target.as_symbol(), &LibraryParser::parse_target},
    {token_use.as_symbol(), &LibraryParser::parse_use}
};

const std::unordered_map<Symbol, void (LibraryParser::*)(const Token& name, const std::shared_ptr<StructuredValue>& definition)> LibraryParser::symbol_parser_methods = {
    {token_constant.as_symbol(), &LibraryParser::parse_constant},
    {token_function.as_symbol(), &LibraryParser::parse_function},
    {token_macro.as_symbol(), &LibraryParser::parse_macro},
    {token_object.as_symbol(), &LibraryParser::parse_object}
};
// clang-format on

LibraryParser::LibraryParser() {
    tokenizer.add_literal(token_in_range);
    tokenizer.add_literal(token_label_offset);
    tokenizer.add_literal(token_object_name);
    tokenizer.add_literal(Token::NAME, ".label_", "0123456789");
    tokenizer.add_literal(Token::colon_minus);
    tokenizer.add_literal(Token::colon_plus);
}

void LibraryParser::parse(Symbol filename, Module* module) {
    this->module = module;

    if (!parse_file(filename)) {
        throw Exception("can't parse library '{}'", filename);
    }
}

void LibraryParser::parse_directive(const Token& directive) {
    auto it = parser_methods.find(directive.as_symbol());
    if (it != parser_methods.end()) {
        (this->*it->second)();
    }
    else {
        auto it_symbol = symbol_parser_methods.find(directive.as_symbol());
        if (it_symbol != symbol_parser_methods.end()) {
            auto name = tokenizer.expect(Token::NAME, TokenGroup::newline);
            (this->*it_symbol->second)(name, StructuredValue::parse(tokenizer));
        }
        else {
            throw LocationException(directive.location, "unknown directive");
        }
    }
}

void LibraryParser::parse_constant(const Token& name, const std::shared_ptr<StructuredValue>& definition) {
    // TODO: implement
    // file->add_constant(std::make_unique<ObjectFile::Constant>(file.get(), name, definition));
}

void LibraryParser::parse_object(const Token& name, const std::shared_ptr<StructuredValue>& definition) {
    // TODO: implement
    // file->add_object(std::make_unique<Object>(file.get(), name, definition));
}

void LibraryParser::parse_pin() {
    auto name = tokenizer.expect(Token::NAME);
    auto address = ExpressionParser(tokenizer).parse();

    // TODO: implement
    // module->pin(name.as_symbol(), address);
}

void LibraryParser::parse_format_version() {
    auto token = tokenizer.expect(Token::VALUE);
    // TODO: implement
}

void LibraryParser::parse_function(const Token& name, const std::shared_ptr<StructuredValue>& definition) {
    // TODO: implement
    // file->add_function(std::make_unique<Function>(file.get(), name, definition));
}

void LibraryParser::parse_target() {
    auto name = tokenizer.expect(Token::STRING, TokenGroup::newline);

    auto target = &Target::get(name.as_symbol());
    Target::set_current_target(target);
    // TODO: implement
    // module->set_target(target);
}

void LibraryParser::parse_use() {
    while (true) {
        auto token = tokenizer.next();
        if (!token.is_name()) {
            if (token && !token.is_newline()) {
                throw LocationException(token.location, "expected newline");
            }
            break;
        }

        // TODO: implement
        // module->mark_used(token.as_symbol());
    }
}

void LibraryParser::parse_macro(const Token& name, const std::shared_ptr<StructuredValue>& definition) {
    // TODO: implement
    // file->add_macro(std::make_unique<Macro>(file.get(), name, definition));
}

void LibraryParser::parse_import() {
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
                throw LocationException(token.location, "expected ','");
            }
            token = tokenizer.next();
        }
        if (!token.is_string()) {
            throw LocationException(token.location, "expected string");
        }
        // file->import(LibraryGetter::global.get(token.as_symbol(), file->name).get());
    }
}
