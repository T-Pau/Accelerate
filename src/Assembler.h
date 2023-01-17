/*
Assembler.h --

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

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "AssemblerObject.h"
#include "CPU.h"
#include "TokenizerFile.h"

class Assembler {
public:
    explicit Assembler(const CPU& cpu): cpu(cpu) {}

    AssemblerObject parse(const std::string& file_name);

private:
    void parse_assignment(const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    void parse_label(const Token& name);

    std::shared_ptr<Node> parse_instruction_argument(const Token& token);

    const CPU& cpu;

    TokenizerFile tokenizer;
    AssemblerObject object;

    static void initialize();
    static bool initialized;
    static symbol_t symbol_opcode;
    static Token token_colon;
    static Token token_equals;
    static Token token_brace_close;
    static Token token_brace_open;
};


#endif // ASSEMBLER_H
