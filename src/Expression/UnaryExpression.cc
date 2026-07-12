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

#include "UnaryExpression.h"

#include <tpau-cpp-kernal/Exception.h>

#include "EvaluationContext.h"
#include "ValueExpression.h"

using namespace tpau::cpp_kernal;

Expression UnaryExpression::create(const Location& location, Operation operation, Expression operand) {
    return *simplify(location, operation, operand, true);
}

std::optional<Expression> UnaryExpression::simplify(const Location& location, Operation operation, const Expression& operand, bool always_create) {
    std::shared_ptr<BaseExpression> node;
    
    if (operation == Operation::PLUS) {
        return operand;
    }
    else if (operand.has_value()) {
        auto value = *operand.value();
        switch (operation) {
            case Operation::PLUS:
                break;

            case Operation::MINUS:
                value = -value;
                break;

            case Operation::BITWISE_NOT:
                value = ~value; // TODO: mask to size
                break;

            case Operation::LOW_BYTE:
                value = value & 0xff;
                break;

            case Operation::HIGH_BYTE:
                value = (value >> 8) & 0xff;
                break;

            case Operation::BANK_BYTE:
                value = (value >> 16) & 0xff;
                break;

            case Operation::NOT:
                value = Value(!value);
                break;
        }

        return ValueExpression::create(location, value);
    }
    else if (always_create) {
        return Expression(std::make_shared<UnaryExpression>(location, operation, operand));
    }
    else {
        return {};
    }
}

std::optional<Expression> UnaryExpression::evaluate_process(const EvaluationContext& context) {
    return simplify(location, operation, operand, false);
}



void UnaryExpression::serialize_sub(std::ostream& stream) const {
    switch (operation) {
        case Operation::BANK_BYTE:
            stream << '^';
            break;

        case Operation::BITWISE_NOT:
            stream << '~';
            break;

        case Operation::HIGH_BYTE:
            stream << '>';
            break;

        case Operation::LOW_BYTE:
            stream << '<';
            break;

        case Operation::MINUS:
            stream << '-';
            break;

        case Operation::NOT:
            stream << '!';
            break;

        case Operation::PLUS:
            break;
    }

    stream << operand;
}

std::optional<Value> UnaryExpression::minimum_value() const {
    switch (operation) {
        case Operation::BANK_BYTE:
        case Operation::HIGH_BYTE:
        case Operation::LOW_BYTE:
            return Value(static_cast<uint64_t>(0));

        case Operation::BITWISE_NOT:
        case Operation::NOT:
            return {};

        case Operation::MINUS: {
            const auto v = operand.maximum_value();
            if (v.has_value()) {
                return -v.value();
            }
            else {
                return {};
            }
        }

        case Operation::PLUS:
            return operand.minimum_value();
    }

    throw Exception("internal error: invalid unary operation {}", static_cast<int>(operation));
}

std::optional<Value> UnaryExpression::maximum_value() const {
    switch (operation) {
        case Operation::BANK_BYTE:
        case Operation::HIGH_BYTE:
        case Operation::LOW_BYTE:
            return Value(static_cast<uint64_t>(0xff));

        case Operation::BITWISE_NOT:
        case Operation::NOT:
            return {};

        case Operation::MINUS: {
            const auto v = operand.minimum_value();
            if (v.has_value()) {
                return -v.value();
            }
            else {
                return {};
            }
        }

        case Operation::PLUS:
            return operand.maximum_value();
    }

    throw Exception("internal error: invalid unary operation {}", static_cast<int>(operation));
}

std::optional<Value::Type> UnaryExpression::type() const {
    const auto operand_type = operand.type();

    switch (operation) {
        case Operation::BANK_BYTE:
        case Operation::HIGH_BYTE:
        case Operation::LOW_BYTE:
            return Value::UNSIGNED;

        case Operation::BITWISE_NOT:
            if (operand_type) {
                return *operand_type;
            }
            else {
                return Value::INTEGER;
            }

        case Operation::NOT:
            return Value::BOOLEAN;

        case Operation::MINUS:
        case Operation::PLUS:
            if (operand_type) {
                return *operand_type;
            }
            else {
                return Value::NUMBER;
            }
    }

    throw Exception("internal error: invalid unary operation {}", static_cast<int>(operation));
}

