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
#include "CPU.h"
#include "TokenizerFile.h"
#include "Object.h"

class Assembler {
public:
    explicit Assembler(const CPU& cpu): cpu(cpu) {}

    ObjectFile parse(const std::string& file_name);

private:
    void parse_assignment(Object::Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    void parse_label(Object::Visibility visibility, const Token& name);
    void parse_section();
    void parse_symbol(Object::Visibility visibility, const Token& name);
    void parse_symbol_body();

    std::shared_ptr<Node> parse_instruction_argument(const Token& token);

    void add_constant(Object::Visibility visibility, const Token& name, std::shared_ptr<Expression> value);
    [[nodiscard]] std::shared_ptr<Expression> get_pc() const;

    static Object::Visibility visibility_value(const Token& token);

    const CPU& cpu;

    symbol_t current_section = 0;
    Object* current_object;
    std::shared_ptr<Environment> file_environment;
    std::shared_ptr<Environment> current_environment;

    TokenizerFile tokenizer;
    ObjectFile object_file;

    static void initialize();
    static bool initialized;

    static symbol_t symbol_opcode;
    static symbol_t symbol_pc;

    static Token token_align;
    static Token token_brace_close;
    static Token token_brace_open;
    static Token token_colon;
    static Token token_curly_brace_close;
    static Token token_curly_brace_open;
    static Token token_data;
    static Token token_equals;
    static Token token_global;
    static Token token_local;
    static Token token_reserve;
    static Token token_section;
};

#endif // ASSEMBLER_H
