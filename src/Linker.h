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

#include "CPU.h"
#include "MemoryMap.h"
#include "ObjectFile.h"

class Linker {
public:
    Linker() = default;
    Linker(MemoryMap map_, CPU cpu): map(std::move(map_)), cpu(std::move(cpu)), memory(map.initialize_memory()) {}

    void add_file(const ObjectFile& file) {program.add_object_file(file);}
    void add_library(ObjectFile library) {libraries.emplace_back(std::move(library));}

    void link();
    void output(const std::string& file_name) const;

private:
    MemoryMap map;
    CPU cpu;

    ObjectFile program;
    std::vector<ObjectFile> libraries;

    bool add_object(Object* object);

    std::unordered_set<Object*> objects;

    Memory memory;

};


#endif // LINKER_H
