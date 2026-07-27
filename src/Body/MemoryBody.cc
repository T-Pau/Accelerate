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

#include "MemoryBody.h"

#include <tpau-cpp-kernal/Exception.h>

using namespace tpau::cpp_kernal;

Body MemoryBody::create(Location location, Expression bank, Expression start_address, Expression end_address) { return Body(std::make_shared<MemoryBody>(location, std::move(bank), std::move(start_address), std::move(end_address))); }

MemoryBody::MemoryBody(Location location_, Expression bank_, Expression start_address_, Expression end_address_) : BodyElement(location_, SizeRange(0, {})), bank(std::move(bank_)), start_address(std::move(start_address_)), end_address(std::move(end_address_)) {
    auto minimum_start = start_address.minimum_value().value_or(Value(uint64_t{0}));
    if (auto maximum_end = end_address.maximum_value()) {
        size_range_.maximum = maximum_end->unsigned_value() - minimum_start.unsigned_value() + 1;
    }

    auto maximum_start = start_address.maximum_value();
    auto minimum_end = end_address.minimum_value();
    if (maximum_start && minimum_end) {
        size_range_.minimum = minimum_end->unsigned_value() - maximum_start->unsigned_value() + 1;
    }
    else {
        size_range_.maximum = {};
    }
}

void MemoryBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".memory ";
    if (!bank.has_value() || bank.value() != Value(uint64_t{0})) {
        stream << bank << ", ";
    }
    stream << start_address << ", " << end_address << std::endl;
}

void MemoryBody::encode(std::string& bytes, const Memory* memory) {
    if (memory == nullptr) {
        throw Exception("can't encode .memory without memory");
    }
    if (!bank.has_value() || !start_address.has_value() || !end_address.has_value()) {
        throw Exception("unresolved address"); // TODO: more details
    }

    auto bank_value = bank.value()->unsigned_value();
    auto start_value = start_address.value()->unsigned_value();
    auto end_value = end_address.value()->unsigned_value();

    bytes += (*memory)[bank_value].data(Range(start_value, end_value - start_value + 1));
}

void MemoryBody::traverse(std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) {
    expression_callback(bank);
    expression_callback(start_address);
    expression_callback(end_address);
}
