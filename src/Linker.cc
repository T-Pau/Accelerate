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
#include "ObjectExpression.h"

void Linker::link() {
    program.evaluate(*program.local_environment);

    for (auto& library: libraries) {
        library.evaluate(*library.local_environment);
        program.evaluate(*library.global_environment);
    }

    std::unordered_set<Object*> new_objects;

    // Collect all objects from main program.
    for (auto object : program.all_objects()) {
        if (add_object(object)) {
            new_objects.insert(object);
        }
    }

    // Collect all referenced objects.
    while (!new_objects.empty()) {
        auto objects = new_objects;
        new_objects.clear();
        for (auto object : objects) {
            for (const auto& expression: object->data) {
                for (auto sub_expression: *expression) {
                    if (sub_expression->type() == Expression::OBJECT) {
                        auto sub_object = dynamic_cast<ObjectExpression*>(sub_expression)->object;
                        if (add_object(sub_object)) {
                            new_objects.insert(sub_object);
                        }
                    }
                }
            }
        }
    }

    // TODO: check for unresolved symbols

    // TODO: place objects

    // resolve object addresses
    auto empty_environment = Environment();
    for (auto& pair : objects_by_section) {
        pair.second->data.evaluate(empty_environment);
    }

    // TODO: copy object data to memory banks
}

void Linker::output(const std::string &file_name) const {
    // TODO: implement
}

bool Linker::add_object(Object *object) {
    return objects_by_section.insert({object->section, object}).second;
}
