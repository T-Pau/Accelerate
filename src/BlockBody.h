/*
BlockBody.h --

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

#ifndef BLOCK_BODY_H
#define BLOCK_BODY_H

#include <vector>

#include "Body.h"

class BlockBody: public BodyElement {
public:
    BlockBody() = default;
    explicit BlockBody(std::vector<Body> block);
    static Body create(const std::vector<Body>& elements);

    [[nodiscard]] std::optional<Body> append_sub(Body body, Body element) override;
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {return std::make_shared<BlockBody>(block);}
    [[nodiscard]] bool empty() const override {return block.empty();}
    void encode(std::string &bytes, const Memory* memory) const override;
    [[nodiscard]] std::optional<Body> evaluated(const EvaluationContext& context) const override;
    [[nodiscard]] std::optional<Body> back() const {if (block.empty()) {return {};} else {return block.back();}}

    void serialize(std::ostream& stream, const std::string& prefix) const override;

private:
    std::vector<Body> block;

    void append_element(const Body& element);
};

#endif // BLOCK_BODY_H
