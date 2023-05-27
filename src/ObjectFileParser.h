/*
ObjectFileParser.h -- 

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

#ifndef OBJECT_FILE_PARSER_H
#define OBJECT_FILE_PARSER_H

#include <string>

#include "FileParser.h"
#include "ObjectFile.h"


class ObjectFileParser: public FileParser {
public:
    ObjectFileParser();

    std::shared_ptr<ObjectFile> parse(Symbol filename);

protected:
    void parse_directive(const Token& directive) override;

private:
    std::shared_ptr<ObjectFile> file;

    void parse_constant();
    void parse_format_version();
    void parse_object();
    void parse_target();

    static Visibility visibility_from_name(const Token& name);

    static void initialize();

    static bool initialized;
    static std::unordered_map<Symbol, void (ObjectFileParser::*)()> parser_methods;
    static Token token_address;
    static Token token_alignment;
    static Token token_constant;
    static Token token_data;
    static Token token_format_version;
    static Token token_global;
    static Token token_local;
    static Token token_object;
    static Token token_reserve;
    static Token token_section;
    static Token token_target;
    static Token token_value;
    static Token token_visibility;
};


#endif // OBJECT_FILE_PARSER_H
