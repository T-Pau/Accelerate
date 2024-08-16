/*
Expression.cc --

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

#include "BaseExpression.h"
#include "EvaluationResult.h"
#include "SizeRange.h"
#include "Tokenizer.h"
#include "VoidExpression.h"

enum LabelExpressionType {
    NAMED,
    NEXT_UNNAMED,
    PREVIOUS_UNNAMED
};

class BinaryExpression;
class Entity;
class EvaluationContext;
class Object;
class ObjectExpression;
class ObjectNameExpression;
class VariableExpression;

class Expression {
public:
    enum BinaryOperation {
        ADD,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        DIVIDE,
        EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,
        LOGICAL_AND,
        LOGICAL_OR,
        MODULO,
        MULTIPLY,
        NOT_EQUAL,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        SUBTRACT
    };
    enum UnaryOperation {
        BANK_BYTE,
        BITWISE_NOT,
        HIGH_BYTE,
        LOW_BYTE,
        MINUS,
        NOT,
        PLUS
    };

    Expression() = default;
    explicit Expression(std::shared_ptr<BaseExpression> expression);
    explicit Expression(Tokenizer& tokenizer);

    // Binary
    Expression(const Location& location, const Expression& left, BinaryOperation operation, const Expression& right);
    // Function
    Expression(const Location& location, Symbol name, const std::vector<Expression>& arguments);
    // Label
    Expression(const Location& location, Symbol object_name, Symbol label_name, const SizeRange& offset = SizeRange(0, {}));
    Expression(const Location& location, const Entity* object, Symbol label_name, const SizeRange& offset = SizeRange(0, {}), const SizeRange& scope_offset = SizeRange(0, {}), bool keep = false);
    Expression(const Location& location, LabelExpressionType type, size_t unnamed_index = std::numeric_limits<size_t>::max(), const SizeRange& offset = SizeRange(0, {}));
    // Object
    explicit Expression(const Location& location, Object* object);
    // Unary
    Expression(const Location& location, UnaryOperation operation, const Expression& operand);
    // Value
    explicit Expression(const Location& location, bool value): Expression(location, Value(value)) {}
    explicit Expression(const Location& location, uint64_t value): Expression(location, Value(value)) {}
    explicit Expression(const Location& location, const Value& value);
    // Variable
    explicit Expression(const Location& location, Symbol name);
    // Variable or Value
    explicit Expression(const Token& value);

    [[nodiscard]] const BinaryExpression* as_binary() const;
    [[nodiscard]] const ObjectExpression* as_object() const;
    [[nodiscard]] const ObjectNameExpression* as_object_name() const;
    [[nodiscard]] const VariableExpression* as_variable() const;
    void collect_objects(std::unordered_set<Object*>& variables) const {expression->collect_objects(variables);}
    bool evaluate(const EvaluationContext& context);
    [[nodiscard]] std::shared_ptr<BaseExpression> get_expression() const {return expression;}
    [[nodiscard]] bool has_value() const {return value().has_value();}
    [[nodiscard]] bool is_binary() const {return as_binary() != nullptr;}
    [[nodiscard]] bool is_object() const {return as_object() != nullptr;}
    [[nodiscard]] bool is_object_name() const {return as_object_name() != nullptr;}
    [[nodiscard]] bool is_unique() const {return expression.use_count() <= 1;}
    [[nodiscard]] bool is_variable() const {return as_variable() != nullptr;}
    [[nodiscard]] const Location& location() const {return expression->location;}
    [[nodiscard]] std::optional<Value> maximum_value() const {return expression->maximum_value();}
    [[nodiscard]] std::optional<Value> minimum_value() const {return expression->minimum_value();}
    void serialize(std::ostream& stream) const;
    [[nodiscard]] std::optional<Value::Type> type() const {return expression->type();}
    [[nodiscard]] std::optional<Value> value() const {return expression->value();}
    [[nodiscard]] Symbol variable_name() const;

    [[nodiscard]] std::optional<Expression> evaluated(const EvaluationContext& context) const;

private:
    std::shared_ptr<BaseExpression> expression = std::make_shared<VoidExpression>(Location());
};

std::ostream& operator<<(std::ostream& stream, const Expression& expression);

#endif // EXPRESSION_H
