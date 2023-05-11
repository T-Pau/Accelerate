/*
BodyElement.h --

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

#ifndef BODY_ELEMENT_H
#define BODY_ELEMENT_H

#include <optional>
#include <utility>

#include "Environment.h"
#include "Memory.h"

class CPU;

class Body;

class BodyElement {
public:
    enum Type {
        DATA,
        IF
    };

    class EvaluationResult {
    public:
        EvaluationResult() = default;
        EvaluationResult(std::shared_ptr<BodyElement> element, uint64_t offset): element(std::move(element)), minimum_offset(offset), maximum_offset(offset) {}
        EvaluationResult(std::shared_ptr<BodyElement> element, uint64_t minimum_offset, uint64_t maximum_offset): element(std::move(element)), minimum_offset(minimum_offset), maximum_offset(maximum_offset) {}
        EvaluationResult(uint64_t minimum_offset, uint64_t maximum_offset): minimum_offset(minimum_offset), maximum_offset(maximum_offset) {}

        std::shared_ptr<BodyElement> element;
        uint64_t minimum_offset = 0;
        uint64_t maximum_offset = 0;
    };

    static std::shared_ptr<BodyElement> append(const std::shared_ptr<BodyElement>& body, const std::shared_ptr<BodyElement>& element);
    static std::shared_ptr<BodyElement> evaluate(std::shared_ptr<BodyElement> element, const Environment& environment);
    static std::shared_ptr<BodyElement> make_unique(std::shared_ptr<BodyElement> element);

    virtual void collect_objects(std::unordered_set<Object*>& objects) const {}
    virtual void encode(std::string& bytes, const Memory* memory) const = 0;
    [[nodiscard]] virtual std::shared_ptr<BodyElement> clone() const = 0;
    [[nodiscard]] virtual bool empty() const = 0;
    [[nodiscard]] virtual EvaluationResult evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const = 0;
    [[nodiscard]] virtual uint64_t maximum_size() const = 0;
    [[nodiscard]] virtual uint64_t minimum_size() const = 0;
    [[nodiscard]] virtual std::optional<uint64_t> size() const = 0;

    virtual void serialize(std::ostream& stream, const std::string& prefix) const = 0;

    [[nodiscard]] virtual std::shared_ptr<BodyElement> append_sub(std::shared_ptr<BodyElement> body, std::shared_ptr<BodyElement> element) {return {};}

    friend class Body;
};

std::ostream& operator<<(std::ostream& stream, const BodyElement& element);
std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BodyElement>& element);

#endif // BODY_ELEMENT_H
