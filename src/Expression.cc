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

#include "Expression.h"

#include "BinaryExpression.h"
#include "ExpressionParser.h"
#include "FunctionExpression.h"
#include "LabelExpression.h"
#include "ObjectExpression.h"
#include "ObjectFileParser.h"
#include "ObjectNameExpression.h"
#include "ParseException.h"
#include "UnaryExpression.h"
#include "ValueExpression.h"
#include "VariableExpression.h"

Expression::Expression(Tokenizer& tokenizer) {
    *this = ExpressionParser(tokenizer).parse();
}

Expression::Expression(const Expression &left, Expression::BinaryOperation operation, const Expression &right): expression(BinaryExpression::create(left, operation, right).expression) {}
Expression::Expression(Expression::UnaryOperation operation, const Expression &operand): expression(UnaryExpression::create(operation, operand).expression) {}
Expression::Expression(const Token& token) {
    if (token == ObjectFileParser::token_object_name) {
        expression = std::make_shared<ObjectNameExpression>(token.location);
    }
    else if (token.is_name() || token == Token::colon_minus || token == Token::colon_plus) {
        expression = std::make_shared<VariableExpression>(token.as_symbol());
    }
    else {
        expression = std::make_shared<ValueExpression>(token);
    }
}

Expression::Expression(Location location, Symbol object_name, Symbol label_name, SizeRange offset): expression(std::make_shared<LabelExpression>(location, object_name, label_name, offset)) {}
Expression::Expression(Location location, const Entity* object, Symbol label_name, SizeRange offset, SizeRange scope_offset, bool keep) {
    *this = LabelExpression::create(location, object, label_name, offset, scope_offset, keep);
}
Expression::Expression(Location location, LabelExpressionType type, size_t unnamed_index, SizeRange offset) {
    *this = LabelExpression::create(location, type, unnamed_index, offset);
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

const ObjectNameExpression *Expression::as_object_name() const {
    return std::dynamic_pointer_cast<ObjectNameExpression>(expression).get();
}

const VariableExpression *Expression::as_variable() const {
    return std::dynamic_pointer_cast<VariableExpression>(expression).get();
}

bool Expression::evaluate(const EvaluationContext& context) {
    try {
        auto new_expression = expression->evaluated(context);
        if (new_expression.has_value()) {
            *this = *new_expression;
        }
        return new_expression.has_value();
    }
    catch (Exception &ex) {
        throw ParseException(expression->location, "%s", ex.what());
    }
}

void Expression::serialize(std::ostream &stream) const {
    if (expression) {
        expression->serialize(stream);
    }
}

Symbol Expression::variable_name() const {
    auto variable = as_variable();
    if (variable) {
        return variable->symbol;
    }
    auto object = as_object();
    if (object) {
        return object->object->name.as_symbol();
    }
    return {};
}

std::ostream& operator<<(std::ostream& stream, const Expression& expression) {
    expression.serialize(stream);
    return stream;
}

std::optional<Expression> Expression::evaluated(const EvaluationContext& context) const {
    return expression->evaluated(context);
}
