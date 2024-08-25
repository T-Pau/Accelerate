/*
Linker.cc -- 

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

#include "Linker.h"

#include <fstream>
#include <ranges>

#include "Assembler.h"
#include "Exception.h"
#include "FileReader.h"
#include "LibraryLinker.h"
#include "ProgramLinker.h"

LibraryLinker* Linker::as_library_linker() {
    return dynamic_cast<LibraryLinker*>(this);
}
ProgramLinker* Linker::as_program_linker() {
    return dynamic_cast<ProgramLinker*>(this);
}


void Linker::link() {
    link_sub();
}


void Linker::link_new() {
    auto entities = UsedEntities{};

    auto new_entities = roots();
    while (!new_entities.empty()) {
        entities.insert(new_entities);
        auto next_entities = UsedEntities{};
        for (auto& [entity, _]: new_entities) {
            auto result = entity->evaluate(EvaluationContext::RESOLVE);
            //next_entities.insert(result.used_entities);
        }
        // TODO: Handle unresolved symbols? Or handle them later when all ifs have been resolved?
        new_entities = std::move(next_entities);
    }

    // sort constants

    // propagate constants
    //   all ifs must be resolved

    // if creating program
    //     place objects
    //     propagate constants
}



bool Linker::add_object(Object *object) {
    return objects.insert(object).second;
}

void Linker::set_target(const Target* new_target) {
    if (!new_target) {
        return;
    }
    if (target) {
        if (new_target != target) {
            throw Exception("can't link files for different targets");
        }
        return;
    }
    target = new_target;
    program->private_environment->add_next(target->object_file->public_environment);
}

void Linker::add_library(std::shared_ptr<ObjectFile> library) {
    program->import(library.get());
    libraries.emplace_back(std::move(library));
}
