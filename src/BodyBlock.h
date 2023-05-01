/*
BodyBlock.h --

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

#ifndef BODY_BLOCK_H
#define BODY_BLOCK_H

#include "BodyElement.h"
#include "Environment.h"

#include <vector>

class BodyBlock: public BodyElement {
public:
    BodyBlock() = default;
    explicit BodyBlock(std::vector<std::shared_ptr<BodyElement>> elements): elements(std::move(elements)) {}

    [[nodiscard]] std::shared_ptr<BodyElement> append_sub(std::shared_ptr<BodyElement> body, std::shared_ptr<BodyElement> element) override;
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<BodyBlock>(elements);}
    [[nodiscard]] bool empty() const override {return elements.empty();}
    void encode(std::string &bytes) const override;
    [[nodiscard]] std::shared_ptr<BodyElement> evaluate(const Environment& environment) const override;
    [[nodiscard]] uint64_t maximum_size() const override;
    [[nodiscard]] uint64_t minimum_size() const override;
    [[nodiscard]] std::optional<uint64_t> size() const override;

    void serialize(std::ostream& stream) const override;

private:
    std::vector<std::shared_ptr<BodyElement>> elements;
};

std::ostream& operator<<(std::ostream& stream, const BodyBlock& block);

#endif // BODY_BLOCK_H
