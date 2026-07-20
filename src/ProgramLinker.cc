/*
Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

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

#include "ProgramLinker.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/Exception.h>
#include <tpau-cpp-kernal/FileReader.h>

#include "Assembler.h"
#include "EvaluationOrder.h"
#include "Scope.h"

using namespace tpau::cpp_kernal;

std::vector<Entity*> ProgramLinker::root_entities() {
    memory = target->map.initialize_memory();

    auto entities = module().explicitly_used_entities();

    // output can't be evaluated until objects have been copied to memory, so just collect its dependencies here
    target->output->resolve();
    entities.insert(entities.end(), target->output->referenced_entities.begin(), target->output->referenced_entities.end());

    // TODO: add explicitly used objects from libraries?

    return entities;
}

void ProgramLinker::link_sub() {
    auto unsorted_objects = entities | std::views::filter([](Entity* entity) { return entity->is<Object>(); }) | std::views::transform([](Entity* entity) { return static_cast<Object*>(entity); });
    auto objects = sorted(unsorted_objects.begin(), unsorted_objects.end());

    for (auto object : objects) {
        if (!object->size_range().size()) {
            DiagnosticOutput::global.error(object->location, "object '{}' has unknown size", object->name);
            if (DiagnosticOutput::global.verbose_error_messages) {
                std::cout << object->body;
            }
            continue;
        }

        if (object->address && object->address.has_value()) {
            auto size = *object->size_range().size();
            auto bank = *object->address->bank();
            auto address = *object->address->address();

            // TODO: validate that object->address is in object->section

            auto range = Range(address, size);
            if (!memory[bank].allocate(range, object->is_reservation() ? Memory::RESERVED : Memory::DATA, 0, range.size)) {
                DiagnosticOutput::global.error(object->location, "fixed space for '{}' (${}, ${}) not free", object->name, range.start, range.end());
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
                DiagnosticOutput::global.error(object->location, "no space left for '{}' ({} bytes) in section '{}'", object->name, *object->size_range().size(), object->section->name);
                continue;
            }
        }
    }

    DiagnosticOutput::global.exit_if_failed();

    for (auto entity : entities) {
        entity->evaluate();
    }

    DiagnosticOutput::global.exit_if_failed();

    for (auto object : objects) {
        if (!object->address || object->address.has_value()) {
            DiagnosticOutput::global.error(object->location, "object '{}' has no address", object->name);
            if (DiagnosticOutput::global.verbose_error_messages) {
                std::cout << object->body;
            }
            continue;
        }
        if (!object->is_reservation()) {
            std::string bytes;
            bytes.reserve(*object->size_range().size());
            object->body.encode(bytes);
            if (bytes.size() != object->size_range().size()) {
                throw LocationException(object->location, "internal error: encoded size ({}) != expected object size ({})", bytes.size(), *object->size_range().size());
            }
            memory[object->address->bank().value()].copy(object->address->address().value(), bytes);
        }
    }
}

void ProgramLinker::output(const std::filesystem::path& file_name) {
#if 0
    auto environment = std::make_shared<Scope>();

    for (const auto& object: objects) {
        if (object->has_address()) {
            environment->add(object->name, Expression(object->location, object->address->address));
        }
    }

    // TODO: support for multiple banks
    auto data_range = memory[0].data_range();

    environment->add(Assembler::token_data_end.as_symbol(), Expression({}, data_range.end()));
    environment->add(Assembler::token_data_size.as_symbol(), Expression({}, data_range.size));
    environment->add(Assembler::token_data_start.as_symbol(), Expression({}, data_range.start));
    environment->add_next(target->object_file->private_environment);
    environment->add_next(program->public_environment);

    EvaluationResult result;
    output_body.evaluate(EvaluationContext(result, EvaluationContext::OUTPUT, environment, target->defines, SizeRange(0)));
    // TODO: process result

    auto bytes = std::string();
    bytes.reserve(output_body.size_range().minimum);

    output_body.encode(bytes, &memory);

    for (const auto& checksum: result.checksums) {
        checksum.compute(bytes);
    }

    auto stream = std::ofstream(file_name, std::ios::binary);
    stream << bytes;
#endif
}

void ProgramLinker::output_symbol_map(const std::filesystem::path& file_name) {
#if 0
    auto sorted_objects = std::vector<Object*>(objects.begin(), objects.end());
    std::ranges::sort(sorted_objects, Object::less_pointers);

    std::unordered_set<const ObjectFile::Constant*> constants;
    std::vector<const ObjectFile::Constant*> sorted_constants;

    program->collect_constants(constants, false);
    for (const auto& object_file: libraries) {
        object_file->collect_constants(constants, true);

        for (const auto& object: object_file->all_objects()) {
            if (object->has_address()) {
                sorted_objects.push_back(object);
            }
        }
    }

    sorted_constants.insert(sorted_constants.end(), constants.begin(), constants.end());
    // TODO: sort by address
    std::ranges::sort(sorted_constants, [](const ObjectFile::Constant* a, const ObjectFile::Constant* b) {
        auto va = a->value.value();
        auto vb = b->value.value();
        if (va && vb) {
            if (*va != *vb) {
                return *va < *vb;
            }
            else {
                return a->name < b->name;
            }
        }
        else if (va) {
            return true;
        }
        else if (vb) {
            return false;
        }
        else {
            return a->name < b->name;
        }
    });

    auto stream = std::ofstream(file_name);

    for (const auto& constant: sorted_constants) {
        auto value = constant->value.value();
        if (value) {
            stream << "constant\t" << *value << "\t" << constant->name << "\n";
        }
    }

    for (const auto& object: sorted_objects) {
        stream << "object\t" << *object->address;
        stream << "\t$" << std::setfill('0') << std::setw(4) << std::hex << *object->size_range().size() << std::dec;
        stream << "\t" << object->name;
        stream << "\t" << object->section->name << "\t" << (object->is_reservation() ? "reserve" : "data") << "\n";
    }
#endif
}

#if 0
UsedEntities ProgramLinker::roots() {
    auto entities = UsedEntities{};
    entities.insert(target->output.get());
    return entities;
}
#endif
