/*
Expression.h -- Abstract Base Class of Expression Nodes

Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <cstddef>

#include "Environment.h"
#include "Node.h"
#include "TokenizerFile.h"
#include "Int.h"

class Expression {
public:
    enum Type {
        BINARY,
        INTEGER,
        UNARY,
        VARIABLE
    };

    [[nodiscard]] virtual Type type() const = 0;
    [[nodiscard]] virtual bool has_value() const {return false;}
    [[nodiscard]] virtual int64_t value() const {return 0;}
    [[nodiscard]] virtual size_t minimum_byte_size() const = 0;
    virtual void replace_variables(symbol_t (*transform)(symbol_t)) = 0;

    [[nodiscard]] size_t byte_size() const {return byte_size_;}
    void set_byte_size(size_t size);

    static std::shared_ptr<Expression> evaluate(std::shared_ptr<Expression> node, const Environment& environment);

    void serialize(std::ostream& stream) const;
    [[nodiscard]] std::vector<symbol_t> get_variables() const;

    [[nodiscard]] virtual std::shared_ptr<Expression> evaluate(const Environment& environment) const = 0;
    [[nodiscard]] virtual std::shared_ptr<Expression> clone() const = 0;

    virtual void collect_variables(std::vector<symbol_t>& variables) const = 0;

    Location location;

protected:
    virtual void serialize_sub(std::ostream& stream) const = 0;

private:
    size_t byte_size_ = 0;
};

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Expression>& node);
std::ostream& operator<<(std::ostream& stream, const Expression& node);

class IntegerExpression: public Expression {
public:
    explicit IntegerExpression(const Token& token);
    explicit IntegerExpression(int64_t value): value_(value) {set_byte_size(Int::minimum_byte_size(value));}

    [[nodiscard]] Type type() const override {return INTEGER;}

    [[nodiscard]] bool has_value() const override {return true;}
    [[nodiscard]] int64_t value() const override {return value_;}
    [[nodiscard]] size_t minimum_byte_size() const override {return Int::minimum_byte_size(value());}
    void replace_variables(symbol_t (*transform)(symbol_t)) override {}

    void collect_variables(std::vector<symbol_t>& variables) const override {}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override {return {};}
    [[nodiscard]] std::shared_ptr<Expression> clone() const override;

    void serialize_sub(std::ostream& stream) const override;

private:
    int64_t value_;
};

class VariableExpression: public Expression {
public:
    explicit VariableExpression(const Token& token);
    explicit VariableExpression(symbol_t symbol): symbol(symbol) {}

    [[nodiscard]] Type type() const override {return VARIABLE;}

    [[nodiscard]] size_t minimum_byte_size() const override {return 0;} // TODO
    void replace_variables(symbol_t (*transform)(symbol_t)) override;

    [[nodiscard]] symbol_t variable() const {return symbol;}

    void collect_variables(std::vector<symbol_t>& variables) const override {variables.emplace_back(symbol);}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override;
    [[nodiscard]] std::shared_ptr<Expression> clone() const override;

    void serialize_sub(std::ostream& stream) const override {stream << SymbolTable::global[symbol];}

private:
    symbol_t symbol;
};

class UnaryExpression: public Expression {
public:
    enum Operation {
        BANK_BYTE,
        BITWISE_NOT,
        HIGH_BYTE,
        LOW_BYTE,
        MINUS,
        PLUS
    };

    UnaryExpression(Operation operation, std::shared_ptr<Expression>operand);
    [[nodiscard]] std::shared_ptr<Expression> static create(Operation operation, std::shared_ptr<Expression> operand, size_t byte_size);

    [[nodiscard]] Type type() const override {return UNARY;}
    [[nodiscard]] size_t minimum_byte_size() const override;
    void replace_variables(symbol_t (*transform)(symbol_t)) override {operand->replace_variables(transform);}

    void collect_variables(std::vector<symbol_t>& variables) const override { operand->collect_variables(variables);}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override;
    [[nodiscard]] std::shared_ptr<Expression> clone() const override;

    void serialize_sub(std::ostream& stream) const override;

private:
    Operation operation;
    std::shared_ptr<Expression> operand;
};



class BinaryExpression: public Expression {
public:
    enum Operation {
        ADD,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        DIVIDE,
        MODULO,
        MULTIPLY,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        SUBTRACT
    };

    BinaryExpression(std::shared_ptr<Expression>  left, Operation operation, std::shared_ptr<Expression> right);
    [[nodiscard]] std::shared_ptr<Expression> static create(const std::shared_ptr<Expression>& left, Operation operation, const std::shared_ptr<Expression>& right, size_t byte_size = 0);

    [[nodiscard]] Type type() const override {return BINARY;}

    [[nodiscard]] size_t minimum_byte_size() const override;
    void replace_variables(symbol_t (*transform)(symbol_t)) override {left->replace_variables(transform); right->replace_variables(transform);}

    void collect_variables(std::vector<symbol_t>& variables) const override {left->collect_variables(variables); right->collect_variables(variables);}

protected:
    [[nodiscard]] std::shared_ptr<Expression> evaluate(const Environment &environment) const override;
    [[nodiscard]] std::shared_ptr<Expression> clone() const override;

    void serialize_sub(std::ostream& stream) const override;

private:
    Operation operation;
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
};

#endif // EXPRESSION_H
