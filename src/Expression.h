//
// Created by Dieter Baron on 10.05.23.
//

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "BaseExpression.h"
#include "VoidExpression.h"

class BinaryExpression;
class Object;
class ObjectExpression;
class VariableExpression;

class Expression {
public:
    enum BinaryOperation {
        ADD,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        DIVIDE,
        LOGICAL_AND,
        LOGICAL_OR,
        MODULO,
        MULTIPLY,
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
        PLUS
    };

    Expression() = default;
    explicit Expression(const std::shared_ptr<BaseExpression>& expression_) {if (expression_) {expression = expression_;}}

    // Binary
    Expression(const Expression& left, BinaryOperation operation, const Expression& right);
    // Function
    Expression(Symbol name, const std::vector<Expression>& arguments);
    // Object
    explicit Expression(Object* object);
    // Unary
    Expression(UnaryOperation operation, const Expression& operand);
    // Value
    explicit Expression(uint64_t value): Expression(Value(value)) {}
    explicit Expression(Value value);
    // Variable
    explicit Expression(Symbol name);
    // Variable or Value
    explicit Expression(const Token& value);

    [[nodiscard]] const BinaryExpression* as_binary() const;
    [[nodiscard]] const ObjectExpression* as_object() const;
    [[nodiscard]] const VariableExpression* as_variable() const;
    void collect_objects(std::unordered_set<Object*>& variables) const {expression->collect_objects(variables);}
    bool evaluate(const Environment& environment);
    [[nodiscard]] std::shared_ptr<BaseExpression> get_expression() const {return expression;}
    [[nodiscard]] bool has_value() const {return value().has_value();}
    [[nodiscard]] bool is_binary() const {return as_binary() != nullptr;}
    [[nodiscard]] bool is_object() const {return as_object() != nullptr;}
    [[nodiscard]] bool is_variable() const {return as_variable() != nullptr;}
    [[nodiscard]] const Location& location() const {return expression->location;}
    [[nodiscard]] std::optional<Value> maximum_value() const {return expression->maximum_value();}
    [[nodiscard]] std::optional<Value> minimum_value() const {return expression->minimum_value();}
    void serialize(std::ostream& stream) const;
    [[nodiscard]] std::optional<Value> value() const {return expression->value();}

    [[nodiscard]] std::optional<Expression> evaluated(const Environment& environment) const {return expression->evaluated(environment);}

private:
    std::shared_ptr<BaseExpression> expression = std::make_shared<VoidExpression>();
};

std::ostream& operator<<(std::ostream& stream, const Expression& expression);

#endif // EXPRESSION_H
