/*
CPU.h -- Definition of a CPU and Its Instruction Set

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

#ifndef CPU_H
#define CPU_H

#include <unordered_map>

#include "Symbol.h"
#include "AddressingMode.h"
#include "ArgumentType.h"
#include "Instruction.h"
#include "AddressingModeMatcher.h"

class CPUParser;

class CPU {
public:
    static const CPU& get(Symbol name, Symbol base = {});
    static const CPU empty;

    [[nodiscard]] bool is_compatible_with(const CPU& other) const; // this has everything from other

    uint64_t byte_order = 0;

    [[nodiscard]] const AddressingMode* addressing_mode(Symbol name) const;
    [[nodiscard]] const ArgumentType* argument_type(Symbol name) const;
    [[nodiscard]] const Instruction* instruction(Symbol name) const;

    [[nodiscard]] std::vector<AddressingModeMatcherResult> match_addressing_modes(const std::vector<std::shared_ptr<Node>>& arguments) const;

    void setup(FileTokenizer& tokenizer) const;

    [[nodiscard]] bool uses_empty_mnemonic() const {return instructions.find(Symbol()) != instructions.end();}
    [[nodiscard]] bool uses_braces() const {return uses_punctuation(Symbol("("));}
    [[nodiscard]] bool uses_punctuation(Symbol symbol) const {return punctuation.find(symbol) != punctuation.end();}

private:
    std::unordered_map<Symbol, AddressingMode> addressing_modes;
    std::unordered_map<Symbol, std::unique_ptr<ArgumentType>> argument_types;
    std::unordered_map<Symbol, Instruction> instructions;
    std::unordered_set<Symbol> reserved_words;
    std::unordered_set<Symbol> punctuation;
    AddressingModeMatcher addressing_mode_matcher;

    void add_addressing_mode(Symbol name, AddressingMode addressing_mode);
    void add_argument_type(Symbol name, std::unique_ptr<ArgumentType> argument_type);
    void add_reserved_word(Symbol word) {reserved_words.insert(word);}
    void add_punctuation(Symbol name) {punctuation.insert(name);}

    friend CPUParser;
};


#endif // CPU_H
