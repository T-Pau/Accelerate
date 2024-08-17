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

#include "CPU.h"

#include <algorithm>

#include "CPUGetter.h"

const CPU CPU::empty = CPU();

void CPU::add_addressing_mode(Symbol name, AddressingMode mode) {
    addressing_modes[name] = std::move(mode);

    auto& addressing_mode = addressing_modes[name];

    addressing_mode.priority = addressing_modes.size();

    size_t notation_index = 0;
    for (const auto& notation: addressing_mode.notations) {
        addressing_mode_matcher.add_notation(name, notation_index, notation, addressing_mode.arguments);
        notation_index += 1;
    }
}


const AddressingMode *CPU::addressing_mode(Symbol name) const {
    auto it = addressing_modes.find(name);
    if (it == addressing_modes.end()) {
        return nullptr;
    }
    else {
        return &it->second;
    }
}


const ArgumentType *CPU::argument_type(Symbol name) const {
    auto it = argument_types.find(name);
    if (it == argument_types.end()) {
        return nullptr;
    }
    else {
        return it->second.get();
    }
}

void CPU::add_argument_type(Symbol name, std::unique_ptr<ArgumentType> argument_type) {
    argument_types[name] = std::move(argument_type);
}

const Instruction *CPU::instruction(Symbol name) const {
    auto it = instructions.find(name);

    if (it == instructions.end()) {
        return nullptr;
    }
    else {
        return &it->second;
    }
}

void CPU::setup(FileTokenizer& tokenizer) const {
    for (const auto& item: reserved_words) {
        tokenizer.add_literal(Token::KEYWORD, item.str());
    }
    for (const auto& item: punctuation) {
        tokenizer.add_literal(Token::PUNCTUATION, item.str());
    }
    for (const auto& [name, _]: instructions) {
        if (!name.empty()) {
            tokenizer.add_literal(Token::INSTRUCTION, name.str());
        }
    }
}

const CPU &CPU::get(Symbol name, Symbol base) {
    return CPUGetter::global.get(name, base);
}

bool CPU::is_compatible_with(const CPU &other) const {
    if (other.byte_order == 0) {
        // empty CPU
        return true;
    }
    if (byte_order != other.byte_order) {
        return false;
    }

    // TODO: compare instructions

    return true;
}


std::vector<AddressingModeMatcherResult> CPU::match_addressing_modes(const std::vector<std::shared_ptr<Node>>& arguments) const {
    auto matches = addressing_mode_matcher.match(arguments);
    auto v = std::vector<AddressingModeMatcherResult>(matches.begin(), matches.end());
    std::ranges::sort(v, [this](const AddressingModeMatcherResult& a, const AddressingModeMatcherResult& b) -> bool {
             return *addressing_mode(a.addressing_mode) < *addressing_mode(b.addressing_mode);
         });
    return v;
}
