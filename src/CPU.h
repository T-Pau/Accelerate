/*
CPU.h -- Definition of a CPU and Its Instruction Set

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

#ifndef CPU_H
#define CPU_H

#include <unordered_map>

#include "SymbolTable.h"
#include "AddressingMode.h"
#include "ArgumentType.h"
#include "Instruction.h"
#include "AddressingModeMatcher.h"

class CPUParser;

class CPU {
public:
    uint64_t byte_order;

    [[nodiscard]] const AddressingMode* addressing_mode(symbol_t name) const;
    [[nodiscard]] const ArgumentType* argument_type(symbol_t name) const;
    [[nodiscard]] const Instruction* instruction(symbol_t name) const;

    [[nodiscard]] std::unordered_set<AddressingModeMatcherResult> match_addressing_modes(const std::vector<std::shared_ptr<Node>>& arguments) const {return addressing_mode_matcher.match(arguments);}

    void setup(TokenizerFile& tokenizer) const;

    [[nodiscard]] bool uses_braces() const {return uses_punctuation(SymbolTable::global["("]);}
    [[nodiscard]] bool uses_punctuation(symbol_t symbol) const {return punctuation.find(symbol) != punctuation.end();}

private:
    std::unordered_map<symbol_t, AddressingMode> addressing_modes;
    std::unordered_map<symbol_t, std::unique_ptr<ArgumentType>> argument_types;
    std::unordered_map<symbol_t, Instruction> instructions;
    std::unordered_set<symbol_t> reserved_words;
    std::unordered_set<symbol_t> punctuation;
    AddressingModeMatcher addressing_mode_matcher;

    void add_addressing_mode(symbol_t name, AddressingMode addressing_mode);
    void add_argument_type(symbol_t name, std::unique_ptr<ArgumentType> argument_type);
    void add_reserved_word(symbol_t word) {reserved_words.insert(word);}
    void add_punctuation(symbol_t name) {punctuation.insert(name);}

    friend CPUParser;
};


#endif // CPU_H
