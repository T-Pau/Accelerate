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

#include <limits>

#include "Int.h"
#include "Exception.h"


Value::Value(int64_t value_, uint64_t default_size): explicit_default_size(default_size) {
    if (value_ < 0) {
        value = value_;
    }
    else {
        value = static_cast<uint64_t>(value_);
    }
}


uint64_t Value::unsigned_value() const {
    if (is_unsigned()) {
        return raw_unsigned_value();
    }
    else {
        throw Exception("can't convert value of type %s to unsigned", type_name().c_str());
    }
}


int64_t Value::signed_value() const {
    switch (type()) {
        case BINARY:
        case BOOLEAN:
        case FLOAT:
        case STRING:
        case VOID:
            throw Exception("can't convert value of type %s to signed", type_name().c_str());

        case SIGNED:
            return raw_signed_value();

        case UNSIGNED:
            if (raw_unsigned_value() > std::numeric_limits<int64_t>::max()) {
                throw Exception("can't convert too large value to signed");
            }
            return static_cast<int64_t>(raw_unsigned_value());
    }
}

Symbol Value::symbol_value() const {
    if (is_string()) {
        return raw_symbol_value();
    }
    else {
        throw Exception("can't convert value of type %s to string", type_name().c_str());
    }
}


double Value::float_value() const {
    switch (type()) {
        case BINARY:
        case BOOLEAN:
        case STRING:
        case VOID:
            throw Exception("can't convert value of type %s to float", type_name().c_str());

        case FLOAT:
            return raw_float_value();

        case SIGNED:
            return static_cast<double>(raw_signed_value());

        case UNSIGNED:
            return static_cast<double>(raw_unsigned_value());
    }
}


bool Value::boolean_value() const {
    switch (type()) {
        case BOOLEAN:
            return raw_boolean_value();

        case FLOAT:
            return raw_float_value() != 0.0;

        case SIGNED:
            return raw_signed_value() != 0;

        case STRING:
            return raw_symbol_value();

        case UNSIGNED:
            return raw_unsigned_value() != 0;

        case BINARY:
        case VOID:
            throw Exception("can't convert value of type %s to boolean", type_name().c_str());
    }
}

Value::Type Value::type() const {
    if (is_binary()) {
        return BINARY;
    }
    else if (is_boolean()) {
        return BOOLEAN;
    }
    else if (is_float()) {
        return FLOAT;
    }
    else if (is_signed()) {
        return SIGNED;
    }
    else if (is_string()) {
        return STRING;
    }
    else if (is_unsigned()) {
        return UNSIGNED;
    }
    else {
        return VOID;
    }
}

std::string Value::type_name() const {
    switch (type()) {
        case BINARY:
            return "binary";
        case BOOLEAN:
            return "boolean";
        case FLOAT:
            return "float";
        case SIGNED:
            return "signed";
        case STRING:
            return "string";
        case UNSIGNED:
            return "unsigned";
        case VOID:
            return "void";
    }
}

std::string Value::binary_value() const {
    if (is_binary()) {
        return raw_binary_value();
    }
    else {
        throw Exception("can't convert value of type %s to binary", type_name().c_str());
    }
}

Value Value::operator+(const Value &other) const {
    if (is_float() || other.is_float()) {
        return Value(float_value() + other.float_value());
    }
    else if (is_unsigned() && other.is_unsigned()) {
        return Value(add_unsigned(raw_unsigned_value(), other.raw_unsigned_value()));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(add_signed(raw_signed_value(), other.raw_signed_value()));
    }
    else if (is_signed() && other.is_unsigned()) {
        auto negated_this = negate_signed(raw_signed_value());
        if (negated_this >= other.raw_unsigned_value()) {
            return Value(other.raw_unsigned_value() - negated_this);
        }
        else {
            return Value(negate_unsigned(negated_this - other.raw_unsigned_value()));
        }
    }
    else if (is_unsigned() && other.is_signed()) {
        auto negated_other = negate_signed(other.raw_signed_value());
        if (negated_other >= raw_unsigned_value()) {
            return Value(raw_unsigned_value() - negated_other);
        }
        else {
            return Value(negate_unsigned(negated_other - raw_unsigned_value()));
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
        if (raw_unsigned_value() >= other.raw_unsigned_value()) {
            return Value(raw_unsigned_value() - other.raw_unsigned_value());
        }
        else {
            return Value(negate_unsigned(other.raw_unsigned_value() - raw_unsigned_value()));
        }
    }
    else if (is_signed() && other.is_signed()) {
        return Value(raw_signed_value() - other.raw_signed_value());
    }
    else if (is_signed() && other.is_unsigned()) {
        return Value(add_signed(raw_signed_value(), negate_unsigned(other.raw_unsigned_value())));
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(add_unsigned(raw_unsigned_value(), negate_signed(other.raw_signed_value())));
    }
    else {
        throw Exception("can't subtract %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

Value Value::operator/(const Value &other) const {
    if ((other.is_float() && other.float_value() == 0.0) || (other.is_unsigned() && other.unsigned_value() == 0)) {
        throw Exception("division by zero");
    }
    if (is_float() || other.is_float()) {
        return Value(float_value() / other.float_value());
    }
    else if (is_unsigned() && other.is_unsigned()) {
        return Value(raw_unsigned_value() / other.raw_unsigned_value());
    }
    else if (is_signed() && other.is_signed()) {
        return Value(raw_signed_value() / other.raw_signed_value());
    }
    else if (is_signed() && other.is_unsigned()) {
        return Value(negate_unsigned(negate_signed(raw_signed_value()) / other.raw_unsigned_value()));
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(negate_unsigned(raw_unsigned_value() / negate_signed(other.raw_signed_value())));
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
        return Value(multiply_unsigned(raw_unsigned_value(), other.raw_unsigned_value()));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(multiply_unsigned(negate_signed(raw_signed_value()), negate_signed(other.raw_signed_value())));
    }
    else if (is_signed() && other.is_unsigned()) {
        return Value(negate_unsigned(multiply_unsigned(negate_signed(raw_signed_value()), other.raw_unsigned_value())));
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(negate_unsigned(multiply_unsigned(raw_unsigned_value(), negate_signed(other.raw_signed_value()))));
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
    if (negated_value > 0) {
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
        return Value(raw_unsigned_value() | other.raw_unsigned_value());
    }
    else {
        throw Exception("can't bitwise or %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}


Value Value::operator&(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(raw_unsigned_value() & other.raw_unsigned_value());
    }
    else {
        throw Exception("can't bitwise and %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}


Value Value::operator^(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(raw_unsigned_value() ^ other.raw_unsigned_value());
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
        return Value(raw_unsigned_value() >> other.raw_unsigned_value());
    }
    else if (is_signed() && other.is_unsigned()) {
        if (other.raw_unsigned_value() > std::numeric_limits<int64_t>::max()) {
            return Value(static_cast<uint64_t>(0));
        }
        else {
            return Value(raw_signed_value() >> static_cast<int64_t>(other.raw_unsigned_value()));
        }
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(shift_left_unsigned(raw_unsigned_value(), negate_signed(other.raw_signed_value())));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(shift_left_signed(raw_signed_value(), negate_signed(other.raw_signed_value())));
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
        return Value(shift_left_unsigned(raw_unsigned_value(), other.raw_unsigned_value()));
    }
    else if (is_signed() && other.is_unsigned()) {
        if (other.raw_unsigned_value() > std::numeric_limits<int64_t>::max()) {
            throw Exception("integer overflow");
        }
        else {
            return Value(shift_left_signed(raw_signed_value(), static_cast<int64_t>(other.raw_unsigned_value())));
        }
    }
    else if (is_unsigned() && other.is_signed()) {
        return Value(raw_unsigned_value() >> negate_signed(other.raw_signed_value()));
    }
    else if (is_signed() && other.is_signed()) {
        return Value(raw_signed_value() >> negate_signed(other.raw_signed_value()));
    }
    else {
        throw Exception("can't shift %s and %s", type_name().c_str(), other.type_name().c_str());
    }
}

bool Value::operator==(const Value &other) const {
    switch (type()) {
        case BINARY:
            return other.is_binary() && raw_binary_value() == other.raw_binary_value();

        case BOOLEAN:
            return other.is_boolean() && raw_boolean_value() == other.raw_boolean_value();

        case FLOAT:
            return other.is_number() && raw_float_value() == other.float_value();

        case SIGNED:
            return other.is_signed() && raw_signed_value() == other.raw_signed_value();

        case STRING:
            return other.is_string() && raw_symbol_value() == other.raw_symbol_value();

        case UNSIGNED:
            return other.is_unsigned() && raw_unsigned_value() == other.raw_unsigned_value();

        case VOID:
            return other.is_void();
    }
}

bool Value::operator<(const Value &other) const {
    switch (type()) {
        case BINARY:
        case BOOLEAN:
        case VOID:
            return false;

        case FLOAT:
            return other.is_number() && raw_float_value() < other.float_value();

        case SIGNED:
            return (other.is_signed() && raw_signed_value() < other.raw_signed_value()) || other.is_unsigned();

        case STRING:
            return other.is_string() && raw_symbol_value() < other.raw_symbol_value();

        case UNSIGNED:
            return other.is_unsigned() && raw_unsigned_value() < other.raw_unsigned_value();
    }
}


bool Value::operator<=(const Value &other) const {
    switch (type()) {
        case BINARY:
        case BOOLEAN:
        case VOID:
            return false;

        case FLOAT:
            return other.is_number() && raw_float_value() <= other.float_value();

        case SIGNED:
            return (other.is_signed() && raw_signed_value() <= other.raw_signed_value()) || other.is_unsigned();

        case STRING:
            return other.is_string() && raw_symbol_value() <= other.raw_symbol_value();

        case UNSIGNED:
            return other.is_unsigned() && raw_unsigned_value() <= other.raw_unsigned_value();
    }
}

Value Value::operator%(const Value &other) const {
    if (is_unsigned() && other.is_unsigned()) {
        return Value(raw_unsigned_value() % other.raw_unsigned_value());
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
            raw_boolean_value() = other.raw_boolean_value();
            break;

        case FLOAT:
            raw_float_value() = std::get<double>(other.value);
            break;

        case SIGNED:
            raw_signed_value() = other.raw_signed_value();
            break;

        case UNSIGNED:
            raw_unsigned_value() = other.raw_unsigned_value();
            break;

        case VOID:
            break;
    }

    return *this;
}*/

Value Value::operator-() const {
    switch (type()) {
        case BINARY:
        case BOOLEAN:
        case STRING:
        case VOID:
            throw Exception("can't negate %s", type_name().c_str());

        case FLOAT:
            return Value(-raw_float_value());

        case SIGNED:
            return Value(negate_signed(raw_signed_value()));

        case UNSIGNED:
            return Value(negate_unsigned(raw_unsigned_value()));
    }
}

Value Value::operator~() const {
    switch (type()) {
        case BINARY: // TODO: implement
        case BOOLEAN:
        case FLOAT:
        case STRING:
        case VOID:
            throw Exception("can't bitwise negate %s", type_name().c_str());

        case SIGNED:
            return Value(~raw_signed_value());

        case UNSIGNED:
            return Value(~raw_unsigned_value());
    }
}

uint64_t Value::default_size() const {
    uint64_t implicit_default_size;

    switch (type()) {
        case BINARY:
            implicit_default_size = raw_binary_value().size();
            break;

        case BOOLEAN:
        case VOID:
        case STRING: // TODO: unknown default size
        case FLOAT:
            implicit_default_size = 0;
            break;

        case SIGNED:
            implicit_default_size = Int::minimum_byte_size(raw_signed_value());
            break;

        case UNSIGNED:
            implicit_default_size = Int::minimum_byte_size(raw_unsigned_value());
            break;
    }

    return std::max(implicit_default_size, explicit_default_size);
}


std::optional<Value> operator+(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (a.has_value() && b.has_value()) {
        return *a + *b;
    }
    else {
        return {};
    }
}


std::optional<Value> operator-(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (a.has_value() && b.has_value()) {
        return *a - *b;
    }
    else {
        return {};
    }
}


std::optional<Value> operator*(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (a.has_value() && b.has_value()) {
        return *a * *b;
    }
    else {
        return {};
    }
}


std::optional<Value> operator/(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (a.has_value() && b.has_value()) {
        return *a / *b;
    }
    else {
        return {};
    }
}

std::optional<Value> operator&&(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (a.has_value() && b.has_value()) {
        return *a && *b;
    }
    else {
        return {};
    }
}

std::optional<Value> operator||(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (a.has_value() && b.has_value()) {
        return *a || *b;
    }
    else {
        return {};
    }
}


bool operator>(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (!a.has_value() || !b.has_value()) {
        return false;
    }
    return *a > *b;
}

bool operator<(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (!a.has_value() || !b.has_value()) {
        return false;
    }
    return *a < *b;
}

bool operator>=(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (!a.has_value() || !b.has_value()) {
        return false;
    }
    return *a >= *b;
}

bool operator<=(const std::optional<Value>& a, const std::optional<Value>& b) {
    if (!a.has_value() || !b.has_value()) {
        return false;
    }
    return *a <= *b;
}

std::string Value::string_value() const {
    switch (type()) {
        case BINARY:
        case BOOLEAN:
        case VOID:
            throw Exception("can't convert %s to string", type_name().c_str());

        case FLOAT:
            return std::to_string(raw_float_value());

        case SIGNED:
            return std::to_string(raw_signed_value());

        case STRING:
            return raw_symbol_value().str();

        case UNSIGNED:
            return std::to_string(raw_unsigned_value());
    }
}