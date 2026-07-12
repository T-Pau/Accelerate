#ifdef IN_XLR8_DATA_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_DATA_BODY_H
#ifndef HAD_XLR8_DATA_BODY_H
#define HAD_XLR8_DATA_BODY_H

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

#include <utility>

#include "Body.h"
#include "Encoder.h"
#include "Expression/Expression.h"

class DataBodyElement {
public:
    DataBodyElement(Expression expression, std::optional<Encoder> encoding): expression(std::move(expression)), encoding(std::move(encoding)) {}

    [[nodiscard]] std::optional<uint64_t> size() const {return size_range().size();}
    [[nodiscard]] SizeRange size_range() const;

    Expression expression;
    std::optional<Encoder> encoding;
};

/**
 * @brief Represents a body containing data. This is used for `.data` statements and encoded instructions.
 */
class DataBody: public BodyElement {
public:
    static Body create(std::vector<DataBodyElement> data) {return Body(std::make_shared<DataBody>(std::move(data)));}

    DataBody() = default;
    explicit DataBody(std::vector<DataBodyElement> data);

    Body appending(const DataBody* body) {return appending(body->data);}
    Body appending(const std::vector<DataBodyElement>& elements) const;
//    void append(Expression expression, std::optional<IntegerEncoder> encoding = {}) {data.emplace_back(std::move(expression), encoding);}
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<DataBody>(data);}
    [[nodiscard]] bool empty() const override {return data.empty();}
    void encode(std::string &bytes, const Memory* memory) const override;
    void serialize(std::ostream &stream, const std::string& prefix) const override;
    void traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) override;

    std::vector<DataBodyElement> data;

protected:
    [[nodiscard]] std::optional<Body> append_sub(const Body& body, const Body& element) override;
};


#endif // HAD_XLR8_DATA_BODY_H
#undef IN_XLR8_DATA_BODY_H
