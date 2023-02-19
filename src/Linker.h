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

#include <unordered_map>
#include <vector>

#include "CPU.h"
#include "MemoryMap.h"
#include "ObjectFile.h"

class Linker {
public:
    Linker() = default;
    Linker(MemoryMap map, CPU cpu): map(std::move(map)), cpu(std::move(cpu)) {}

    void add_file(ObjectFile file) {files.emplace_back(std::move(file));}
    void add_library(ObjectFile library) {libraries.emplace_back(std::move(library));}

    void link();
    void output(const std::string& file_name) const;

private:
    class LinkerObject {
    public:
        LinkerObject(bool imported, size_t source_index, std::shared_ptr<Object> object): imported(imported), source_index(source_index), object(std::move(object)) {}

        bool imported;
        size_t source_index;
        std::shared_ptr<Object> object;
    };

    MemoryMap map;
    CPU cpu;

    std::vector<ObjectFile> files;
    std::vector<ObjectFile> libraries;

    std::vector<LinkerObject> objects;

    std::unordered_map<symbol_t, LinkerObject*> objects_by_section;
};


#endif // LINKER_H
