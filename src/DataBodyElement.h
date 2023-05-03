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

#include "BodyElement.h"

#include <utility>
#include "Encoding.h"
#include "Expression.h"

class DataBodyElement: public BodyElement {
public:
    class Datum {
    public:
        Datum(std::shared_ptr<Expression> expression, std::optional<Encoding> encoding): expression(std::move(expression)), encoding(encoding) {}

        [[nodiscard]] std::optional<uint64_t> size() const;
        [[nodiscard]] uint64_t maximum_size() const {return size().value_or(8);}
        [[nodiscard]] uint64_t minimum_size() const {return size().value_or(1);}

        std::shared_ptr<Expression> expression;
        std::optional<Encoding> encoding;
    };

    DataBodyElement() = default;
    explicit DataBodyElement(std::vector<Datum> data): data(std::move(data)) {}

    void append(const std::shared_ptr<Expression>& expression, std::optional<Encoding> encoding = {}) {data.emplace_back(expression, encoding);}
    [[nodiscard]] std::shared_ptr<BodyElement> append_sub(std::shared_ptr<BodyElement> body, std::shared_ptr<BodyElement> element) override;
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<DataBodyElement>(data);}
    [[nodiscard]] bool empty() const override {return data.empty();}
    void encode(std::string &bytes) const override;
    [[nodiscard]] EvaluationResult evaluate(const Environment &environment, uint64_t minimum_offset, uint64_t maximum_offset) const override;
    [[nodiscard]] uint64_t maximum_size() const override;
    [[nodiscard]] uint64_t minimum_size() const override;
    [[nodiscard]] std::optional<uint64_t> size() const override;

    void serialize(std::ostream &stream) const override;

    std::vector<Datum> data;
};


#endif // DATA_BODY_ELEMENT_H
