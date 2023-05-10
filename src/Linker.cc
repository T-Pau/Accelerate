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
#if 0
    while (!new_objects.empty()) {
        auto current_objects = new_objects;
        new_objects.clear();
        for (auto object: current_objects) {
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
#endif

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
        for (const auto& block: object->section->blocks) {
            auto address = memory[block.bank].allocate(block.range, object->is_reservation() ? Memory::RESERVED : Memory::DATA, object->alignment, object->size);
            if (address.has_value()) {
                object->bank = block.bank;
                object->address = address.value();
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

    auto empty_environment = Environment();
    for (auto object: objects) {
        try {
            object->evaluate(empty_environment);
            std::string bytes;
            object->body.encode(bytes);
            memory[object->bank.value()].copy(object->address.value(), bytes);
        }
        catch (Exception& ex) {
            FileReader::global.error(Location(), "can't evaluate '%s': %s", object->name.as_string().c_str(), ex.what());
        }
    }
}

void Linker::output(const std::string &file_name) {
    auto environment = Environment();

    for (const auto& object: objects) {
        if (object->has_address()) {
            environment.add(object->name.as_symbol(), std::make_shared<ValueExpression>(object->address.value()));
        }
    }

    // TODO: support for multiple banks
    auto data_range = memory[0].data_range();

    environment.add(TargetParser::token_data_end.as_symbol(), std::make_shared<ValueExpression>(data_range.end()));
    environment.add(TargetParser::token_data_size.as_symbol(), std::make_shared<ValueExpression>(data_range.size));
    environment.add(TargetParser::token_data_start.as_symbol(), std::make_shared<ValueExpression>(data_range.start));

    auto stream = std::ofstream(file_name);

#if 0
    for (const auto& element: target.output_elements) {
        auto arguments = element.arguments;
        arguments.evaluate(environment);
        switch (element.type) {
            case OutputElement::DATA:
                stream << arguments.bytes(target.cpu->byte_order);
                break;

            case OutputElement::MEMORY: {
                uint64_t bank = 0;
                size_t index = 0;
                if (arguments.expressions.size() == 3) {
                    bank = arguments.expressions[0]->value().unsigned_value();
                    index += 1;
                }
                uint64_t start = arguments.expressions[index]->value().unsigned_value();
                uint64_t end = arguments.expressions[index + 1]->value().unsigned_value();

                stream << memory[bank].data(Range(start, end - start));
                break;
            }
        }
    }
#endif
}

bool Linker::add_object(Object *object) {
    return objects.insert(object).second;
}
