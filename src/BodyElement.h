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

#include "Environment.h"
#include "Memory.h"
#include "SizeRange.h"
#include "EvaluationContext.h"

class Body;
class CPU;

class BodyElement {
public:
    BodyElement() = default;
    explicit BodyElement(const SizeRange& size_range): size_range_(size_range) {}
    virtual ~BodyElement() = default;

    [[nodiscard]] SizeRange size_range() const {return size_range_;}
    [[nodiscard]] std::optional<uint64_t> size() const {return size_range().size();}

    virtual void collect_objects(std::unordered_set<Object*>& objects) const {}
    [[nodiscard]] virtual std::shared_ptr<BodyElement> clone() const = 0;
    [[nodiscard]] virtual bool empty() const = 0;
    virtual void encode(std::string& bytes, const Memory* memory) const = 0;
    [[nodiscard]] virtual std::optional<Body> evaluated(const EvaluationContext& context) const = 0;
    virtual void serialize(std::ostream& stream, const std::string& prefix) const = 0;

    [[nodiscard]] virtual std::optional<Body> append_sub(const Body& body, const Body& element);

    friend class Body;

protected:
    SizeRange size_range_ = SizeRange(0);
};

std::ostream& operator<<(std::ostream& stream, const BodyElement& element);
std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BodyElement>& element);

#endif // BODY_ELEMENT_H
