/*
ExpressionNode.h -- Abstract Base Class of Expression Nodes

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

#ifndef EXPRESSION_NODE_H
#define EXPRESSION_NODE_H

#include <cstddef>

#include "Node.h"
#include "TokenizerFile.h"

class ExpressionNode : public Node {
public:
    [[nodiscard]] virtual size_t byte_size() const = 0;
    [[nodiscard]] virtual size_t minimum_size() const = 0;

    static std::shared_ptr<ExpressionNode> parse(Tokenizer& tokenizer);

    // serialize to file
    // serialize as bytes
};


class ExpressionNodeInteger: public ExpressionNode {
public:
    explicit ExpressionNodeInteger(const Token& token);
    explicit ExpressionNodeInteger(int64_t value): value(value) {}

    [[nodiscard]] Type type() const override {return INTEGER;}
    [[nodiscard]] size_t byte_size() const override;
    [[nodiscard]] size_t minimum_size() const override;
    
    int64_t value;
};

class ExpressionNodeVariable: public ExpressionNode {
public:
    explicit ExpressionNodeVariable(const Token& token);
    explicit ExpressionNodeVariable(symbol_t symbol): symbol(symbol) {}

    [[nodiscard]] Type type() const override {return VARIABLE;}

    [[nodiscard]] size_t byte_size() const override {return 0;} // TODO
    [[nodiscard]] size_t minimum_size() const override {return 0;} // TODO

private:
    symbol_t symbol;
    int64_t value = 0;
};
#endif // EXPRESSION_NODE_H
