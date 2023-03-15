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

#include <fstream>
#include "Linker.h"
#include "ObjectExpression.h"
#include "FileReader.h"

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
        for (auto object: objects) {
            for (const auto &expression: object->data) {
                for (auto sub_expression: *expression) {
                    if (sub_expression->type() == Expression::OBJECT) {
                        auto sub_object = dynamic_cast<ObjectExpression *>(sub_expression)->object;
                        if (add_object(sub_object)) {
                            new_objects.insert(sub_object);
                        }
                    }
                }
            }
        }
    }

    for (auto object: objects) {
        if (!object->bank.has_value() || object->address.has_value()) {
            // TODO: unresolved symbol
            continue;
        }
    }

    // TODO: sort objects
    for (auto object: objects) {
        if (object->size == 0) {
            FileReader::global.error({}, "object '%s' has unknown size", object->name.as_string().c_str());
            continue;
        }
        std::cout << object->name.as_string() << " = ";
        auto section = map.section(object->section);
        for (const auto& block: section->blocks) {
            auto address = memory[block.bank].allocate(block.range, object->is_reservation() ? Memory::RESERVED : Memory::DATA, object->alignment, object->size);
            if (address.has_value()) {
                object->bank = block.bank;
                object->address = address.value();
                break;
            }
        }
        if (!object->address.has_value()) {
            FileReader::global.error({}, "no space left in section '%s'", SymbolTable::global[section->name].c_str());
            continue;
        }
        std::cout << object->address.value() << std::endl;
        memory[object->bank.value()].debug_blocks(std::cout);
    }

    if (FileReader::global.had_error()) {
        return;
    }

    auto empty_environment = Environment();
    for (auto object: objects) {
        object->data.evaluate(empty_environment);
        memory[object->bank.value()].copy(object->address.value(), object->data.bytes(cpu.byte_order));
    }
}

void Linker::output(const std::string &file_name) {
    // TODO: implement properly
    auto stream = std::ofstream(file_name);

    auto data_range = memory[0].data_range();

    if (!data_range.empty()) {
        std::string start_address;

        Int::encode(start_address, static_cast<int64_t >(data_range.start), 2, cpu.byte_order);
        stream << start_address;
        stream << memory[0].data(data_range);
    }
}

bool Linker::add_object(Object *object) {
    return objects.insert(object).second;
}
