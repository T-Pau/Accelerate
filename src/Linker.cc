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
#include "ValueExpression.h"
#include "Exception.h"
#include "TargetParser.h"

void Linker::link() {
    for (auto& library: libraries) {
        library->evaluate();
        program->import(library.get());
    }

    program->evaluate();
    program->evaluate();

    std::unordered_set<Object*> new_objects;

    // Collect all objects from main program.
    for (auto object : program->all_objects()) {
        if (add_object(object)) {
            new_objects.insert(object);
        }
    }

    // Collect all referenced objects.
    while (!new_objects.empty()) {
        auto current_objects = new_objects;
        new_objects.clear();
        for (auto object: current_objects) {
            auto used_objects = std::unordered_set<Object*>();
            object->body.collect_objects(used_objects);
            for (const auto& used_object: used_objects) {
                if (add_object(used_object)) {
                    new_objects.insert(used_object);
                }
            }
        }
    }

    for (auto object: objects) {
        if (!object->address.has_value()) {
            // TODO: unresolved symbol
            continue;
        }
    }

    // TODO: sort objects
    for (auto object: objects) {
        if (!object->size_range().size()) {
            FileReader::global.error({}, "object '%s' has unknown size", object->name.as_string().c_str());
            continue;
        }
        for (const auto& block: object->section->blocks) {
            auto address = memory[block.bank].allocate(block.range, object->is_reservation() ? Memory::RESERVED : Memory::DATA, object->alignment, *object->size_range().size());
            if (address.has_value()) {
                object->address = {block.bank, *address};
                break;
            }
        }
        if (!object->address.has_value()) {
            FileReader::global.error({}, "no space left in section '%s'", object->section->name.c_str());
            continue;
        }
    }

    if (FileReader::global.had_error()) {
        return;
    }

    for (auto object: objects) {
        try {
            object->evaluate();
            std::string bytes;
            object->body.encode(bytes);
            memory[object->address->bank].copy(object->address->address, bytes);
        }
        catch (Exception& ex) {
            FileReader::global.error(Location(), "can't evaluate '%s': %s", object->name.as_string().c_str(), ex.what());
        }
    }
}

void Linker::output(const std::string &file_name) {
    auto environment = std::make_shared<Environment>();

    for (const auto& object: objects) {
        if (object->has_address()) {
            environment->add(object->name.as_symbol(), Expression(object->address->address));
        }
    }

    // TODO: support for multiple banks
    auto data_range = memory[0].data_range();

    environment->add(TargetParser::token_data_end.as_symbol(), Expression(data_range.end()));
    environment->add(TargetParser::token_data_size.as_symbol(), Expression(data_range.size));
    environment->add(TargetParser::token_data_start.as_symbol(), Expression(data_range.start));

    auto body = target->output;
    EvaluationResult result;
    body.evaluate(EvaluationContext(result, true, environment));
    // TODO: process result

    auto bytes = std::string();
    bytes.reserve(body.size_range().minimum);

    body.encode(bytes, &memory);

    auto stream = std::ofstream(file_name);
    stream << bytes;
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
    memory = target->map.initialize_memory();
    Encoding::default_byte_order = target->cpu->byte_order;
}

void Linker::add_library(std::shared_ptr<ObjectFile> library) {
    if (mode != COMPILE) {
        program->import(library.get());
    }
    libraries.emplace_back(std::move(library));
}
