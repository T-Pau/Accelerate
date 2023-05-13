/*
Linker.h -- 

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

#ifndef LINKER_H
#define LINKER_H

#include <unordered_set>
#include <vector>

#include "Target.h"
#include "ObjectFile.h"

class Linker {
public:
    Linker(): target(Target::empty) {}
    explicit Linker(const Target& target): target(target), memory(target.map.initialize_memory()) {Encoding::default_byte_order = target.cpu->byte_order;}

    void add_file(const std::shared_ptr<ObjectFile>& file) {program->add_object_file(file);}
    void add_library(std::shared_ptr<ObjectFile> library) {libraries.emplace_back(std::move(library));}

    void link();
    void output(const std::string& file_name);

    const Target& target;

private:

    std::shared_ptr<ObjectFile> program = std::make_shared<ObjectFile>();
    std::vector<std::shared_ptr<ObjectFile>> libraries;

    bool add_object(Object* object);

    std::unordered_set<Object*> objects;

    Memory memory;
};

#endif // LINKER_H
