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

#include "ObjectFile.h"
#include "Target.h"

class LibraryLinker;
class ProgramLinker;

class Linker {
  public:
    Linker() = default;
    explicit Linker(const Target* target_) {set_target(target_);}
    virtual ~Linker() = default;

    [[nodiscard]] LibraryLinker* as_library_linker();
    [[nodiscard]] ProgramLinker* as_program_linker();
    [[nodiscard]] bool is_library_linker() {return as_library_linker();}
    [[nodiscard]] bool is_program_linker() {return as_program_linker();}

    void add_file(const std::shared_ptr<ObjectFile>& file) {program->add_object_file(file);}

    void add_library(std::shared_ptr<ObjectFile> library);
    void set_target(const Target* new_target);

    void link();
    virtual void output(const std::string& file_name) = 0;

    const Target* target = nullptr;
    std::shared_ptr<ObjectFile> program = std::make_shared<ObjectFile>();

  protected:
    virtual void link_sub() = 0;
    virtual std::unordered_set<Entity*> roots() = 0;

    bool add_object(Object* object);

    std::vector<std::shared_ptr<ObjectFile>> libraries;
    std::unordered_set<Object*> objects;
};

#endif // LINKER_H
