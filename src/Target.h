/*
Target.h --

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

#ifndef TARGET_H
#define TARGET_H

#include <vector>

#include "CPU.h"
#include "MemoryMap.h"
#include "Output.h"
#include "StringEncoding.h"

class ObjectFile;

class Target {
public:
    Target();
    explicit Target(Symbol name): name(name) {}

    static void clear_current_target() {current_target = &empty;};
    static const Target& get(Symbol name);
    static const Target& get(const std::string& name) {return get(Symbol(name));}
    static void set_current_target(const Target* target) {current_target = target;}
    static const Target empty;
    static const Target* current_target;

    [[nodiscard]] bool is_compatible_with(const Target& other) const; // this has everything from other
    [[nodiscard]] const StringEncoding* string_encoding(Symbol name) const;

    Symbol name;
    const CPU* cpu = &CPU::empty;
    MemoryMap map;
    std::unordered_map<Symbol, StringEncoding> string_encodings;
    std::unordered_set<Symbol> defines;
    const StringEncoding* default_string_encoding{};

    std::shared_ptr<ObjectFile> object_file;
    std::unique_ptr<Output> output;

    std::string extension = "bin";
};

#endif // TARGET_H
