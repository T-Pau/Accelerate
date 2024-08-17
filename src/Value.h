/*
Value.h --

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

#ifndef VALUE_H
#define VALUE_H

#include <cstdint>
#include <optional>
#include <ostream>
#include <variant>

#include "Symbol.h"

class Value {
public:
    enum Type {
        BINARY,
        BOOLEAN,
        FLOAT,
        INTEGER,
        NUMBER,
        SIGNED,
        STRING,
        UNSIGNED,
        VOID
    };

    explicit Value() = default;
    explicit Value(uint64_t value, uint64_t default_size = 0): value{value}, explicit_default_size{default_size} {}
    explicit Value(int64_t value, uint64_t default_size = 0);
    explicit Value(bool value): value{value} {}
    explicit Value(double value): value{value} {}
    explicit Value(const std::string& value): value{value} {}
    explicit Value(Symbol value): value{value} {}

    static std::string type_name(Type type);

    [[nodiscard]] bool is_binary() const {return value && std::holds_alternative<std::string>(*value);}
    [[nodiscard]] bool is_boolean() const {return value && std::holds_alternative<bool>(*value);}
    [[nodiscard]] bool is_float() const {return value && std::holds_alternative<double>(*value);}
    [[nodiscard]] bool is_integer() const {return is_signed() || is_unsigned();}
    [[nodiscard]] bool is_number() const {return is_integer() || is_float();}
    [[nodiscard]] bool is_signed() const {return value && std::holds_alternative<int64_t>(*value);}
    [[nodiscard]] bool is_string() const {return value && std::holds_alternative<Symbol>(*value);}
    [[nodiscard]] bool is_unsigned() const {return value && std::holds_alternative<uint64_t>(*value);}
    [[nodiscard]] bool is_void() const {return !value;}

    [[nodiscard]] Type type() const;
    [[nodiscard]] std::string type_name() const {return type_name(type());}
    [[nodiscard]] std::string binary_value() const;
    [[nodiscard]] bool boolean_value() const;
    [[nodiscard]] double float_value() const;
    [[nodiscard]] int64_t signed_value() const;
    [[nodiscard]] std::string string_value() const;
    [[nodiscard]] Symbol symbol_value() const;
    [[nodiscard]] uint64_t unsigned_value() const;
    [[nodiscard]] std::optional<uint64_t> default_size() const;

    explicit operator bool() const {return boolean_value();}

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const {return !(*this == other);}
    bool operator<(const Value& other) const;
    bool operator>(const Value& other) const {return other < *this;}
    bool operator<=(const Value& other) const;
    bool operator>=(const Value& other) const {return other <= *this;}

    Value operator-() const;
    Value operator~() const;
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator|(const Value& other) const;
    Value operator&(const Value& other) const;
    Value operator&(uint64_t other) const {return *this & Value(other);}
    Value operator^(const Value& other) const;
    Value operator%(const Value& other) const;
    Value operator<<(const Value& other) const;
    Value operator>>(const Value& other) const;
    Value operator>>(uint64_t other) const {return *this >> Value(other);}
    Value operator&&(const Value& other) const;
    Value operator||(const Value& other) const;
    Value& operator+=(const Value& other) {*this = *this + other; return *this;}
    Value& operator-=(const Value& other) {*this = *this - other; return *this;}
    Value& operator/=(const Value& other) {*this = *this / other; return *this;}
    Value& operator*=(const Value& other) {*this = *this * other; return *this;}

private:
    std::optional<std::variant<bool, double, int64_t, uint64_t, Symbol, std::string>> value;
    uint64_t explicit_default_size{0};

    [[nodiscard]] auto raw_binary_value() const {return std::get<std::string>(*value);}
    [[nodiscard]] auto raw_boolean_value() const {return std::get<bool>(*value);}
    [[nodiscard]] auto raw_float_value() const {return std::get<double>(*value);}
    [[nodiscard]] auto raw_signed_value() const {return std::get<int64_t>(*value);}
    [[nodiscard]] auto raw_symbol_value() const {return std::get<Symbol>(*value);}
    [[nodiscard]] auto raw_unsigned_value() const {return std::get<uint64_t>(*value);}

    [[nodiscard]] static uint64_t negate_signed(int64_t value);
    [[nodiscard]] static int64_t negate_unsigned(uint64_t value);
    [[nodiscard]] static uint64_t add_unsigned(uint64_t a, uint64_t b);
    [[nodiscard]] static int64_t add_signed(int64_t a, int64_t b);
    [[nodiscard]] static uint64_t multiply_unsigned(uint64_t a, uint64_t b);
    [[nodiscard]] static int64_t shift_left_signed(int64_t a, uint64_t b);
    [[nodiscard]] static uint64_t shift_left_unsigned(uint64_t a, uint64_t b);
};

template<>
struct std::hash<Value> {
    std::size_t operator()(Value const &value) const noexcept {
        switch (value.type()) {
            case Value::BINARY:
                return std::hash<std::string>{}(value.binary_value());
            case Value::BOOLEAN:
                return std::hash<bool>{}(value.boolean_value());
            case Value::FLOAT:
                return std::hash<double>{}(value.float_value());
            case Value::SIGNED:
                return std::hash<::int64_t>{}(value.signed_value());
            case Value::STRING:
                return std::hash<Symbol>{}(value.symbol_value());
            case Value::UNSIGNED:
                return std::hash<::uint64_t>{}(value.unsigned_value());
            case Value::VOID:
            case Value::NUMBER:
            case Value::INTEGER:
                return 0;
        }
        return 0;
    }
};

std::optional<Value> operator+(const std::optional<Value>& a, const std::optional<Value>& b);
std::optional<Value> operator-(const std::optional<Value>& a, const std::optional<Value>& b);
std::optional<Value> operator*(const std::optional<Value>& a, const std::optional<Value>& b);
std::optional<Value> operator/(const std::optional<Value>& a, const std::optional<Value>& b);
std::optional<Value> operator&&(const std::optional<Value>& a, const std::optional<Value>& b);
std::optional<Value> operator||(const std::optional<Value>& a, const std::optional<Value>& b);


bool operator>(const std::optional<Value>& a, const std::optional<Value>& b);
bool operator<(const std::optional<Value>& a, const std::optional<Value>& b);
bool operator>=(const std::optional<Value>& a, const std::optional<Value>& b);
bool operator<=(const std::optional<Value>& a, const std::optional<Value>& b);

#endif // VALUE_H
