/*
Value.cc -- 

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

#include "Value.h"

#include "Int.h"
#include "Exception.h"


Value::Value(int64_t value) {
    if (value < 0) {
        type_ = SIGNED;
        signed_value_ = value;
    }
    else {
        type_ = UNSIGNED;
        unsigned_value_ = static_cast<uint64_t>(value);
    }
}


uint64_t Value::unsigned_value() const {
    switch (type()) {
        case BOOLEAN:
        case FLOAT:
        case SIGNED:
        case VOID:
            throw Exception("can't convert value of type %s to unsigned", type_name().c_str());

        case UNSIGNED:
            return unsigned_value_;
    }
}


int64_t Value::signed_value() const {
    switch (type()) {
        case BOOLEAN:
        case FLOAT:
        case VOID:
            throw Exception("can't convert value of type %s to signed", type_name().c_str());

        case SIGNED:
            return signed_value_;

        case UNSIGNED:
            if (unsigned_value_ > std::numeric_limits<int64_t>::max()) {
                throw Exception("can't convert too large value to signed");
            }
            return static_cast<int64_t>(unsigned_value_);
    }
}


double Value::float_value() const {
    switch (type()) {
        case BOOLEAN:
        case VOID:
            throw Exception("can't convert value of type %s to float", type_name().c_str());

        case FLOAT:
            return float_value_;

        case SIGNED:
            return static_cast<double>(signed_value_);

        case UNSIGNED:
            return static_cast<double>(unsigned_value_);
    }
}


bool Value::boolean_value() const {
    switch (type()) {
        case BOOLEAN:
            return boolean_value_;

        case FLOAT:
            return float_value_ != 0.0;

        case SIGNED:
            return signed_value_ != 0;

        case UNSIGNED:
            return unsigned_value_ != 0;

        case VOID:
            throw Exception("can't convert value of type %s to boolean", type_name().c_str());
    }
}


std::string Value::type_name() const {
    switch (type()) {
        case BOOLEAN:
            return "boolean";
        case FLOAT:
            return "float";
        case SIGNED:
            return "signed";
        case UNSIGNED:
            return "unsigned";
        case VOID:
            return "void";
    }
}

Value Value::operator+(const Value &other) const {
    if (is_float() || other.is_float()) {
        return Value(float_value() + other.float_value());
    }
    else if (is_unsigned() && other.is_unsigned()) {
        return Value(add_unsigned(unsigned_value_, other.unsigned_value_));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(add_signed(signed_value_, other.signed_value_));
    }
    else if (is_signed() && other.is_unsigned()) {
        auto negated_this = negate_signed(signed_value_);
        if (negated_this >= other.unsigned_value_) {
            return Value(other.unsigned_value_ - negated_this);
        }
        else {
            return Value(negate_unsigned(negated_this - other.unsigned_value_));
        }
    }
    else if (is_unsigned() && other.is_signed()) {
        auto negated_other = negate_signed(other.signed_value_);
        if (negated_other >= unsigned_value_) {
            return Value(unsigned_value_ - negated_other);
        }
        else {
            return Value(negate_unsigned(negated_other - unsigned_value_));
        }
    }
    else {
        throw Exception("can't add %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

Value Value::operator-(const Value &other) const {
    if (is_float() || other.is_float()) {
        return Value(float_value() - other.float_value());
    }
    else if (is_unsigned() && other.is_unsigned()) {
        if (unsigned_value_ >= other.unsigned_value_) {
            return Value(unsigned_value_ - other.unsigned_value_);
        }
        else {
            return Value(negate_unsigned(other.unsigned_value_ - unsigned_value_));
        }
    }
    else if (is_signed() && other.is_signed()) {
        return Value(signed_value_ - other.signed_value_);
    }
    else if (is_signed() && other.is_unsigned()) {
        return Value(add_signed(signed_value_, negate_unsigned(other.unsigned_value_)));
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(add_unsigned(unsigned_value_, negate_signed(other.signed_value_)));
    }
    else {
        throw Exception("can't subtract %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

Value Value::operator/(const Value &other) const {
    if (is_float() || other.is_float()) {
        return Value(float_value() / other.float_value());
    }
    else if (is_unsigned() && other.is_unsigned()) {
        return Value(unsigned_value_ / other.unsigned_value_);
    }
    else if (is_signed() && other.is_signed()) {
        return Value(signed_value_ / other.signed_value_);
    }
    else if (is_signed() && other.is_unsigned()) {
        return Value(negate_unsigned(negate_signed(signed_value_) / other.unsigned_value_));
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(negate_unsigned(unsigned_value_ / negate_signed(other.signed_value_)));
    }
    else {
        throw Exception("can't divide %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}


Value Value::operator*(const Value &other) const {
    if (is_float() || other.is_float()) {
        return Value(float_value() * other.float_value());
    }
    else if (is_unsigned() && other.is_unsigned()) {
        return Value(multiply_unsigned(unsigned_value_, other.unsigned_value_));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(multiply_unsigned(negate_signed(signed_value_), negate_signed(other.signed_value_)));
    }
    else if (is_signed() && other.is_unsigned()) {
        return Value(negate_unsigned(multiply_unsigned(negate_signed(signed_value_), other.unsigned_value_)));
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(negate_unsigned(multiply_unsigned(unsigned_value_, negate_signed(other.signed_value_))));
    }
    else {
        throw Exception("can't multiply %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}


uint64_t Value::negate_signed(int64_t value) {
    if (value == std::numeric_limits<int64_t>::min()) {
        return static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1;
    }
    else {
        return static_cast<uint64_t>(-value);
    }
}

int64_t Value::negate_unsigned(uint64_t value) {
    auto negated_value = -static_cast<int64_t>(value);
    if (negated_value >= 0) {
        throw Exception("integer overflow");
    }
    return negated_value;
}

uint64_t Value::add_unsigned(uint64_t a, uint64_t b) {
    auto value = a + b;
    if (value < a) {
        throw Exception("integer overflow");
    }
    return value;
}

int64_t Value::add_signed(int64_t a, int64_t b) {
    auto value = a + b;
    if (value > a) {
        throw Exception("integer overflow");
    }
    return value;
}

uint64_t Value::multiply_unsigned(uint64_t a, uint64_t b) {
    auto value = a * b;
    if (value < a) {
        throw Exception("integer overflow");
    }
    return value;
}

Value Value::operator|(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(unsigned_value_ | other.unsigned_value_);
    }
    else {
        throw Exception("can't bitwise or %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}


Value Value::operator&(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(unsigned_value_ & other.unsigned_value_);
    }
    else {
        throw Exception("can't bitwise and %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}


Value Value::operator^(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(unsigned_value_ ^ other.unsigned_value_);
    }
    else {
        throw Exception("can't bitwise exclusive or %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

Value Value::operator&&(const Value &other) const {
    return Value(boolean_value() && other.boolean_value());
}

Value Value::operator||(const Value &other) const {
    return Value(boolean_value() || other.boolean_value());
}

Value Value::operator>>(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(unsigned_value_ >> other.unsigned_value_);
    }
    else if (is_signed() && other.is_unsigned()) {
        if (other.unsigned_value_ > std::numeric_limits<int64_t>::max()) {
            return Value(static_cast<uint64_t>(0));
        }
        else {
            return Value(signed_value_ >> static_cast<int64_t>(other.unsigned_value_));
        }
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(shift_left_unsigned(unsigned_value_, negate_signed(other.signed_value_)));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(shift_left_signed(signed_value_, negate_signed(other.signed_value_)));
    }
    else {
        throw Exception("can't shift %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

uint64_t Value::shift_left_unsigned(uint64_t a, uint64_t b) {
    // TODO: overflow check
    return a << b;
}

int64_t Value::shift_left_signed(int64_t a, uint64_t b) {
    if (b > std::numeric_limits<int64_t>::max()) {
        return -1;
    }
    else {
        // TODO: overflow check
        return a << static_cast<int64_t>(b);
    }
}

Value Value::operator<<(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(shift_left_unsigned(unsigned_value_, other.unsigned_value_));
    }
    else if (is_signed() && other.is_unsigned()) {
        if (other.unsigned_value_ > std::numeric_limits<int64_t>::max()) {
            throw Exception("integer overflow");
        }
        else {
            return Value(shift_left_signed(signed_value_, static_cast<int64_t>(other.unsigned_value_)));
        }
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(unsigned_value_ >> negate_signed(other.signed_value_));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(signed_value_ >> negate_signed(other.signed_value_));
    }
    else {
        throw Exception("can't shift %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

bool Value::operator==(const Value &other) const {
    switch (type()) {
        case BOOLEAN:
            return other.is_boolean() && boolean_value_ == other.boolean_value_;

        case FLOAT:
            return other.is_number() && float_value_ == other.float_value();

        case SIGNED:
            return other.is_signed() && signed_value_ == other.signed_value_;

        case UNSIGNED:
            return other.is_unsigned() && unsigned_value_ == other.unsigned_value_;

        case VOID:
            return other.is_void();
    }
}

bool Value::operator<(const Value &other) const {
    switch (type()) {
        case BOOLEAN:
        case VOID:
            return false;

        case FLOAT:
            return other.is_number() && float_value_ < other.float_value();

        case SIGNED:
            return (other.is_signed() && signed_value_ < other.signed_value_) || other.is_unsigned();

        case UNSIGNED:
            return other.is_unsigned() && unsigned_value_ < other.unsigned_value_;
    }
}


bool Value::operator<=(const Value &other) const {
    switch (type()) {
        case BOOLEAN:
        case VOID:
            return false;

        case FLOAT:
            return other.is_number() && float_value_ <= other.float_value();

        case SIGNED:
            return (other.is_signed() && signed_value_ <= other.signed_value_) || other.is_unsigned();

        case UNSIGNED:
            return other.is_unsigned() && unsigned_value_ <= other.unsigned_value_;
    }
}

size_t Value::minimum_byte_size() const {
    switch (type()) {
        case BOOLEAN:
        case FLOAT:
        case VOID:
            return 0;

        case SIGNED:
            return Int::minimum_byte_size(signed_value_);

        case UNSIGNED:
            return Int::minimum_byte_size(unsigned_value_);
    }
}

Value Value::operator%(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(unsigned_value_ % other.unsigned_value_);
    }
    // TODO: signed
    else {
        throw Exception("can't compute modulus of %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

/*Value &Value::operator=(const Value &other) {
    type_ = other.type();
    switch (other.type()) {
        case BOOLEAN:
            boolean_value_ = other.boolean_value_;
            break;

        case FLOAT:
            float_value_ = other.float_value_;
            break;

        case SIGNED:
            signed_value_ = other.signed_value_;
            break;

        case UNSIGNED:
            unsigned_value_ = other.unsigned_value_;
            break;

        case VOID:
            break;
    }

    return *this;
}*/

Value Value::operator-() const {
    switch (type()) {
        case BOOLEAN:
        case VOID:
            throw Exception("can't negate %s", type_name().c_str());

        case FLOAT:
            return Value(-float_value_);

        case SIGNED:
            return Value(negate_signed(signed_value_));

        case UNSIGNED:
            return Value(negate_unsigned(unsigned_value_));
    }
}

Value Value::operator~() const {
    switch (type()) {
        case BOOLEAN:
        case FLOAT:
        case VOID:
            throw Exception("can't bitwise negate %s", type_name().c_str());

        case SIGNED:
            return Value(~signed_value_);

        case UNSIGNED:
            return Value(~unsigned_value_);
    }
}
