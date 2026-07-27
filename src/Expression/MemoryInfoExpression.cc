/*
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

#include "Expression/MemoryInfoExpression.h"

#include "Entity/Output.h"
#include "Expression/ValueExpression.h"

Expression MemoryInfoExpression::create(const Location& location, Output* output, InfoType info_type, uint64_t bank) { return Expression(std::make_shared<MemoryInfoExpression>(location, output, info_type, bank)); }

std::optional<Expression> MemoryInfoExpression::evaluate(const EvaluationContext& context) {
    if (!output || !output->memory) {
        return {};
    }

    return ValueExpression::create(location, *value());
}

bool MemoryInfoExpression::has_value() const { return output && output->memory; }

std::optional<Value> MemoryInfoExpression::value() const {
    if (!output || !output->memory) {
        return {};
    }

    switch (info_type) {
        case InfoType::MEMORY_SIZE:
            return Value{(*output->memory)[bank].data_range().size};
        case InfoType::MEMORY_START:
            return Value{(*output->memory)[bank].data_range().start};
        case InfoType::MEMORY_END:
            return Value{(*output->memory)[bank].data_range().end()};
    }

    throw LocationException(location, "unknown memory info type");
}

void MemoryInfoExpression::serialize_sub(std::ostream& stream) const {
    if (bank == 0) {
        // TODO: use token names instead of hard-coded strings
        switch (info_type) {
            case InfoType::MEMORY_SIZE:
                stream << ".memory_size";
                return;
            case InfoType::MEMORY_START:
                stream << ".memory_start";
                return;
            case InfoType::MEMORY_END:
                stream << ".memory_end";
                return;
        }
    }
    else {
        stream << ".memory_info(";
        switch (info_type) {
            case InfoType::MEMORY_SIZE:
                stream << "size";
                break;
            case InfoType::MEMORY_START:
                stream << "start";
                break;
            case InfoType::MEMORY_END:
                stream << "end";
                break;
        }
        if (bank != 0) {
            stream << ", " << bank;
        }
        stream << ")";
    }
}
