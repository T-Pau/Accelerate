//
// Created by Dieter Baron on 10.05.23.
//

#include "Expression.h"

#include "BinaryExpression.h"
#include "FunctionExpression.h"
#include "UnaryExpression.h"
#include "ValueExpression.h"
#include "VariableExpression.h"
#include "ObjectExpression.h"

Expression::Expression(const Expression &left, Expression::BinaryOperation operation, const Expression &right): expression(BinaryExpression::create(left, operation, right).expression) {}
Expression::Expression(Expression::UnaryOperation operation, const Expression &operand): expression(UnaryExpression::create(operation, operand).expression) {}
Expression::Expression(const Token& token) {
    if (token.is_name()) {
        expression = std::make_shared<VariableExpression>(token.as_symbol());
    }
    else {
        expression = std::make_shared<ValueExpression>(token);
    }
}
Expression::Expression(Symbol name): expression(std::make_shared<VariableExpression>(name)) {}
Expression::Expression(Value value): expression(std::make_shared<ValueExpression>(value)) {}
Expression::Expression(Symbol name, const std::vector<Expression>& arguments): expression(FunctionExpression::create(name, arguments).expression) {}
Expression::Expression(Object* object): expression(ObjectExpression::create(object).expression) {}

const BinaryExpression *Expression::as_binary() const {
    return std::dynamic_pointer_cast<BinaryExpression>(expression).get();
}

const ObjectExpression *Expression::as_object() const {
    return std::dynamic_pointer_cast<ObjectExpression>(expression).get();
}

const VariableExpression *Expression::as_variable() const {
    return std::dynamic_pointer_cast<VariableExpression>(expression).get();
}

bool Expression::evaluate(const Environment &environment) {
    auto new_expression = expression->evaluated(environment);
    if (new_expression.has_value()) {
        *this = *new_expression;
    }
    return new_expression.has_value();
}

void Expression::serialize(std::ostream &stream) const {
    if (expression) {
        expression->serialize(stream);
    }
}


std::ostream& operator<<(std::ostream& stream, const Expression& expression) {
    expression.serialize(stream);
    return stream;
}
