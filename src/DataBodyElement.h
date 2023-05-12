/*
DataBodyElement.h --

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

#ifndef DATA_BODY_ELEMENT_H
#define DATA_BODY_ELEMENT_H


#include <utility>

#include "Body.h"
#include "Encoding.h"
#include "Expression.h"

class DataBodyElement: public BodyElement {
public:
    class Datum {
    public:
        Datum(Expression expression, std::optional<Encoding> encoding): expression(std::move(expression)), encoding(encoding) {}

        [[nodiscard]] std::optional<uint64_t> size() const;
        [[nodiscard]] SizeRange size_range() const {return SizeRange(size().value_or(1), size().value_or(8));}

        Expression expression;
        std::optional<Encoding> encoding;
    };

    DataBodyElement() = default;
    explicit DataBodyElement(std::vector<Datum> data);

    void append(Expression expression, std::optional<Encoding> encoding = {}) {data.emplace_back(std::move(expression), encoding);}
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<DataBodyElement>(data);}
    void collect_objects(std::unordered_set<Object*> &objects) const override;
    [[nodiscard]] bool empty() const override {return data.empty();}
    void encode(std::string &bytes, const Memory* memory) const override;
    [[nodiscard]] std::optional<Body> evaluated(const Environment &environment, const SizeRange& offset) const override;

    void serialize(std::ostream &stream, const std::string& prefix) const override;

    std::vector<Datum> data;

protected:
    [[nodiscard]] std::optional<Body> append_sub(Body body, Body element) override;
};


#endif // DATA_BODY_ELEMENT_H
