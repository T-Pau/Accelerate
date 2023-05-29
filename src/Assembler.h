/*
Assembler.h --

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

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "ObjectFile.h"
#include "Target.h"
#include "FileTokenizer.h"
#include "Object.h"
#include "LabelBody.h"

class Assembler {
public:
    explicit Assembler(const Target* target): target(target) {}

    std::shared_ptr<ObjectFile> parse(Symbol file_name);

    static Symbol symbol_opcode;

private:
    static Visibility visibility_value(const Token& token);

    void add_constant(Visibility visibility, const Token& name, Expression value);
    void parse_assignment(Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_section();
    void parse_symbol(Visibility visibility, const Token& name);
    void parse_target();

    const Target* target;

    Symbol current_section;
    std::shared_ptr<Environment> file_environment;

    FileTokenizer tokenizer;
    std::shared_ptr<ObjectFile> object_file;

    static void initialize();
    static bool initialized;

    static Token token_address;
    static Token token_align;
    static Token token_data;
    static Token token_global;
    static Token token_local;
    static Token token_reserve;
    static Token token_section;
    static Token token_target;
};

#endif // ASSEMBLER_H
