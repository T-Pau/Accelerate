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

#include "CPU.h"

void CPU::add_addressing_mode(symbol_t name, AddressingMode mode) {
    addressing_modes[name] = std::move(mode);

    const auto& addressing_mode = addressing_modes[name];

    for (const auto& notation: addressing_mode.notations) {
        addressing_mode_matcher.add_notation(name, notation, addressing_mode.arguments);
    }
}


const AddressingMode *CPU::addressing_mode(symbol_t name) const {
    auto it = addressing_modes.find(name);
    if (it == addressing_modes.end()) {
        return nullptr;
    }
    else {
        return &it->second;
    }
}


const ArgumentType *CPU::argument_type(symbol_t name) const {
    auto it = argument_types.find(name);
    if (it == argument_types.end()) {
        return nullptr;
    }
    else {
        return it->second.get();
    }
}

void CPU::add_argument_type(symbol_t name, std::unique_ptr<ArgumentType> argument_type) {
    argument_types[name] = std::move(argument_type);
}

const Instruction *CPU::instruction(symbol_t name) const {
    auto it = instructions.find(name);

    if (it == instructions.end()) {
        return nullptr;
    }
    else {
        return &it->second;
    }
}

void CPU::setup(TokenizerFile& tokenizer) const {
    for (const auto& item: reserved_words) {
        tokenizer.add_literal(Token::KEYWORD, SymbolTable::global[item]);
    }
    for (const auto& item: punctuation) {
        tokenizer.add_literal(Token::PUNCTUATION, SymbolTable::global[item]);
    }
    for (const auto& pair: instructions) {
        tokenizer.add_literal(Token::INSTRUCTION, SymbolTable::global[pair.first]);
    }
}

