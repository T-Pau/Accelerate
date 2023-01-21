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

#include "Environment.h"
#include "Node.h"
#include "TokenizerFile.h"

class ExpressionNode : public Node {
public:
    enum SubType {
        ADD,
        BANK_BYTE,
        BITWISE_AND,
        BITWISE_NOT,
        BITWISE_OR,
        BITWISE_XOR,
        DIVIDE,
        HIGH_BYTE,
        INTEGER,
        LOW_BYTE,
        MINUS,
        MODULO,
        MULTIPLY,
        PLUS,
        SIZE,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        SUBTRACT,
        VARIABLE
    };

    [[nodiscard]] virtual SubType subtype() const = 0;
    [[nodiscard]] virtual size_t byte_size() const = 0;
    [[nodiscard]] virtual size_t minimum_size() const = 0;

    [[nodiscard]] Type type() const override {return EXPRESSION;}

    static std::shared_ptr<ExpressionNode> evaluate(std::shared_ptr<ExpressionNode> node, const Environment& environment);

    // serialize to file
    // serialize as bytes

    [[nodiscard]] virtual std::shared_ptr<ExpressionNode> evaluate(const Environment& environment) const = 0;
    [[nodiscard]] std::shared_ptr<ExpressionNode> static create_binary(const std::shared_ptr<ExpressionNode>& left, SubType operation, const std::shared_ptr<ExpressionNode>& right);
    [[nodiscard]] std::shared_ptr<ExpressionNode> static create_unary(SubType operation, std::shared_ptr<ExpressionNode> operand);
};


class ExpressionNodeInteger: public ExpressionNode {
public:
    explicit ExpressionNodeInteger(const Token& token);
    explicit ExpressionNodeInteger(int64_t value): value(value) {}

    [[nodiscard]] SubType subtype() const override {return INTEGER;}

    [[nodiscard]] size_t byte_size() const override;
    [[nodiscard]] size_t minimum_size() const override;

    [[nodiscard]] int64_t as_int() const {return value;}

protected:
    [[nodiscard]] std::shared_ptr<ExpressionNode> evaluate(const Environment &environment) const override {return {};}

    int64_t value;
};

class ExpressionNodeVariable: public ExpressionNode {
public:
    explicit ExpressionNodeVariable(const Token& token);
    explicit ExpressionNodeVariable(symbol_t symbol): symbol(symbol) {}

    [[nodiscard]] SubType subtype() const override {return VARIABLE;}

    [[nodiscard]] size_t byte_size() const override {return 0;} // TODO
    [[nodiscard]] size_t minimum_size() const override {return 0;} // TODO

protected:
    [[nodiscard]] std::shared_ptr<ExpressionNode> evaluate(const Environment &environment) const override;

private:
    symbol_t symbol;
};

class ExpressionNodeUnary: public ExpressionNode {
public:
    ExpressionNodeUnary(SubType operation, std::shared_ptr<ExpressionNode>operand);

    [[nodiscard]] SubType subtype() const override {return operation;}
    [[nodiscard]] size_t byte_size() const override {return 0;} // TODO
    [[nodiscard]] size_t minimum_size() const override {return 0;} // TODO

protected:
    [[nodiscard]] std::shared_ptr<ExpressionNode> evaluate(const Environment &environment) const override;

private:
    SubType operation;
    std::shared_ptr<ExpressionNode> operand;
};



class ExpressionNodeBinary: public ExpressionNode {
public:
    ExpressionNodeBinary(std::shared_ptr<ExpressionNode>  left, SubType operation, std::shared_ptr<ExpressionNode> right);
    [[nodiscard]] SubType subtype() const override {return operation;}

    [[nodiscard]] size_t byte_size() const override {return 0;} // TODO
    [[nodiscard]] size_t minimum_size() const override {return 0;} // TODO

protected:
    [[nodiscard]] std::shared_ptr<ExpressionNode> evaluate(const Environment &environment) const override;

private:
    SubType operation;
    std::shared_ptr<ExpressionNode> left;
    std::shared_ptr<ExpressionNode> right;
};

#endif // EXPRESSION_NODE_H
