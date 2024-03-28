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

#include "Assembler.h"
#include <algorithm>
#include <fstream>

#include "Exception.h"
#include "FileReader.h"
#include "ObjectExpression.h"
#include "Unresolved.h"
#include "ValueExpression.h"

void Linker::link() {
    for (auto& library: libraries) {
        library->evaluate();
        program->import(library.get());
    }

    program->resolve_defaults();
    program->evaluate();
    program->evaluate();
    program->evaluate();

    Unresolved unresolved;
    if (!program->check_unresolved(unresolved)) {
        unresolved.report();
        throw Exception();
    }

    Target::set_current_target(target);
    target->object_file->import(program.get());
    target->object_file->resolve_defaults();
    target->object_file->evaluate();
    target->object_file->evaluate();

    if (!target->object_file->check_unresolved(unresolved)) {
        unresolved.report();
        throw Exception();
    }

    EvaluationResult result;
    auto environment = std::make_shared<Environment>();
    environment->add_next(target->object_file->private_environment);
    environment->add_next(program->public_environment);
    auto context = EvaluationContext{result, EvaluationContext::ENTITY, environment};
    output_body = target->output;
    output_body.evaluate(context);
    result.unresolved_variables.erase(Assembler::token_data_end.as_symbol());
    result.unresolved_variables.erase(Assembler::token_data_size.as_symbol());
    result.unresolved_variables.erase(Assembler::token_data_start.as_symbol());
    unresolved.clear();
    unresolved.add(Token(Token::NAME, ".output"), result);
    unresolved.report();

    if (FileReader::global.had_error()) {
        throw Exception();
    }

    std::unordered_set<Object*> new_objects = result.used_objects;
    target->object_file->collect_explicitly_used_objects(new_objects);
    // TODO: warn/error if no used objects?
    objects = new_objects;

    // Collect all referenced objects.
    while (!new_objects.empty()) {
        auto current_objects = new_objects;
        new_objects.clear();
        for (auto object: current_objects) {
            auto used_objects = std::unordered_set<Object*>();
            for (const auto& used_object: object->referenced_objects) {
                if (add_object(used_object)) {
                    new_objects.insert(used_object);
                }
            }
        }
    }

    auto sorted_objects = std::vector<Object*>(objects.begin(), objects.end());
    std::sort(sorted_objects.begin(), sorted_objects.end(), Object::less_pointers);
    for (auto object: sorted_objects) {
        if (!object->size_range().size()) {
            FileReader::global.error({}, "object '%s' has unknown size", object->name.as_string().c_str());
            if (FileReader::global.verbose_error_messages) {
                std::cout << object->body;
            }
            continue;
        }
        if (object->address) {
            // TODO: validate that object->address is in object->section
            auto range = Range(object->address->address, *object->size_range().size());
            if (!memory[object->address->bank].allocate(range, object->is_reservation() ? Memory::RESERVED : Memory::DATA, 0, range.size)) {
                FileReader::global.error({}, "fixed space (%llu, %llu) not free", range.start, range.end());
            }
        }
        else {
            for (const auto& block : object->section->blocks) {
                auto address = memory[block.bank].allocate(block.range, object->is_reservation() ? Memory::RESERVED : Memory::DATA, object->alignment, *object->size_range().size());
                if (address.has_value()) {
                    object->address = {block.bank, *address};
                    break;
                }
            }
            if (!object->address) {
                FileReader::global.error({}, "no space left in section '%s'", object->section->name.c_str());
                continue;
            }
        }
    }

    if (FileReader::global.had_error()) {
        return;
    }

    Target::set_current_target(target);

    for (auto object: objects) {
        try {
            object->evaluate();
            std::string bytes;
            object->body.encode(bytes);
            memory[object->address->bank].copy(object->address->address, bytes);
        }
        catch (Exception& ex) {
            FileReader::global.error(Location(), "can't encode '%s': %s", object->name.as_string().c_str(), ex.what());
            if (FileReader::global.verbose_error_messages) {
                std::cout << object->body;
            }
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

    environment->add(Assembler::token_data_end.as_symbol(), Expression(data_range.end()));
    environment->add(Assembler::token_data_size.as_symbol(), Expression(data_range.size));
    environment->add(Assembler::token_data_start.as_symbol(), Expression(data_range.start));
    environment->add_next(target->object_file->private_environment);
    environment->add_next(program->public_environment);

    EvaluationResult result;
    output_body.evaluate(EvaluationContext(result, EvaluationContext::STANDALONE, environment, target->defines, SizeRange()));
    // TODO: process result

    auto bytes = std::string();
    bytes.reserve(output_body.size_range().minimum);

    output_body.encode(bytes, &memory);

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
    program->private_environment->add_next(target->object_file->public_environment);
    memory = target->map.initialize_memory();
    IntegerEncoder::default_byte_order = target->cpu->byte_order;
}

void Linker::add_library(std::shared_ptr<ObjectFile> library) {
    program->import(library.get());
    libraries.emplace_back(std::move(library));
}

void Linker::output_symbol_map(const std::string& file_name) {
    auto sorted_objects = std::vector<Object*>(objects.begin(), objects.end());
    std::sort(sorted_objects.begin(), sorted_objects.end(), Object::less_pointers);

    auto stream = std::ofstream(file_name);

    for (const auto& object: sorted_objects) {
        stream << "object\t" << *object->address;
        stream << "\t$" << std::setfill('0') << std::setw(4) << std::hex << *object->size_range().size() << std::dec;
        stream << "\t" << object->name.as_symbol();
        stream << "\t" << object->section->name << "\t" << (object->is_reservation() ? "reserve" : "data") << "\n";
    }
}
