#ifdef IN_XLR8_LIBRARY_PARSER_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_LIBRARY_PARSER_H
#ifndef HAD_XLR8_LIBRARY_PARSER_H
#define HAD_XLR8_LIBRARY_PARSER_H

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

#include "FileParser.h"
#include "Module.h"

// TODO: this should probably become a LibraryParser and create a Module.

class LibraryParser : public FileParser {
  public:
    LibraryParser();

    void parse(Symbol filename, Module* module);

    static const Token token_in_range;
    static const Token token_label_offset;
    static const Token token_object_name;

  protected:
    void parse_directive(const Token& directive) override;

  private:
    Module* module{};

    void parse_constant(const Token& name, const std::shared_ptr<StructuredValue>& definition);
    void parse_format_version();
    void parse_function(const Token& name, const std::shared_ptr<StructuredValue>& definition);
    void parse_import();
    void parse_macro(const Token& name, const std::shared_ptr<StructuredValue>& definition);
    void parse_object(const Token& name, const std::shared_ptr<StructuredValue>& definition);
    void parse_pin();
    void parse_target();
    void parse_use();

    static const std::unordered_map<Symbol, void (LibraryParser::*)()> parser_methods;
    static const std::unordered_map<Symbol, void (LibraryParser::*)(const Token& name, const std::shared_ptr<StructuredValue>& definition)> symbol_parser_methods;

    static const Token token_constant;
    static const Token token_format_version;
    static const Token token_function;
    static const Token token_import;
    static const Token token_macro;
    static const Token token_object;
    static const Token token_pin;
    static const Token token_target;
    static const Token token_use;
};

#endif // HAD_XLR8_LIBRARY_PARSER_H
#undef IN_XLR8_LIBRARY_PARSER_H
