/*
UnaryExpression.cc -- 

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

#include "ValueExpression.h"

std::shared_ptr<Expression> UnaryExpression::create(Operation operation, std::shared_ptr<Expression> operand) {
    std::shared_ptr<Expression> node;

    if (operation == PLUS) {
        return operand;
    }
    else if (operand->has_value()) {
        auto value = operand->value().value();
        switch (operation) {
            case PLUS:
                break;

            case MINUS:
                value = -value;
                break;

            case BITWISE_NOT:
                value = ~value; // TODO: mask to size
                break;

            case LOW_BYTE:
                value = value & 0xff;
                break;

            case HIGH_BYTE:
                value = (value >> 8) & 0xff;
                break;

            case BANK_BYTE:
                value = (value >> 16) & 0xff;
                break;
        }

        node = std::make_shared<ValueExpression>(value);
    }
    else {
        node = std::make_shared<UnaryExpression>(operation, operand);
    }

    return node;
}


std::shared_ptr<Expression> UnaryExpression::evaluate(const Environment &environment) const {
    auto new_operand = operand->evaluate(environment);

    if (!new_operand) {
        return {};
    }

    return create(operation, new_operand ? new_operand : operand);
}




void UnaryExpression::serialize_sub(std::ostream &stream) const {
    switch (operation) {
        case BANK_BYTE:
            stream << '^';
            break;

        case BITWISE_NOT:
            stream << '~';
            break;

        case HIGH_BYTE:
            stream << '>';
            break;

        case LOW_BYTE:
            stream << '<';
            break;

        case MINUS:
            stream << '-';
            break;

        case PLUS:
            break;
    }

    stream << operand;
}


std::optional<Value> UnaryExpression::minimum_value() const {
    switch (operation) {
        case BANK_BYTE:
        case HIGH_BYTE:
        case LOW_BYTE:
            return Value(static_cast<uint64_t>(0));

        case BITWISE_NOT:
            return {};

        case MINUS: {
            auto v = operand->maximum_value();
            if (v.has_value()) {
                return -v.value();
            }
            else {
                return {};
            }
        }

        case PLUS:
            return operand->minimum_value();
    }
}


std::optional<Value> UnaryExpression::maximum_value() const {
    switch (operation) {
        case BANK_BYTE:
        case HIGH_BYTE:
        case LOW_BYTE:
            return Value(static_cast<uint64_t>(0xff));

        case BITWISE_NOT:
            return {};

        case MINUS: {
            auto v = operand->minimum_value();
            if (v.has_value()) {
                return -v.value();
            }
            else {
                return {};
            }
        }

        case PLUS:
            return operand->maximum_value();
    }
}
