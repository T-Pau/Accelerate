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


class Value {
public:
    enum Type {
        BOOLEAN,
        FLOAT,
        SIGNED,
        UNSIGNED,
        VOID
    };
    
    explicit Value(): type_(VOID), unsigned_value_(0) {}
    explicit Value(uint64_t value, uint64_t default_size = 0): type_(UNSIGNED), unsigned_value_(value), explicit_default_size(default_size) {}
    explicit Value(int64_t value, uint64_t default_size = 0);
    explicit Value(bool value): type_(BOOLEAN), boolean_value_(value) {}
    explicit Value(double value): type_(FLOAT), float_value_(value) {}
    Value(const Value&other) = default;

//    Value& operator=(const Value& other);

    [[nodiscard]] bool is_boolean() const {return type() == BOOLEAN;}
    [[nodiscard]] bool is_float() const {return type() == FLOAT;}
    [[nodiscard]] bool is_integer() const {return type() == SIGNED || type() == UNSIGNED;}
    [[nodiscard]] bool is_number() const {return is_integer() || is_float();}
    [[nodiscard]] bool is_signed() const {return type() == SIGNED;}
    [[nodiscard]] bool is_unsigned() const {return type() == UNSIGNED;}
    [[nodiscard]] bool is_void() const {return type() == VOID;}

    [[nodiscard]] Type type() const {return type_;}
    [[nodiscard]] std::string type_name() const;
    [[nodiscard]] bool boolean_value() const;
    [[nodiscard]] double float_value() const;
    [[nodiscard]] int64_t signed_value() const;
    [[nodiscard]] uint64_t unsigned_value() const;
    [[nodiscard]] uint64_t default_size() const;

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

private:
    Type type_;
    union {
        bool boolean_value_;
        double float_value_;
        int64_t signed_value_;
        uint64_t unsigned_value_;
    };
    uint64_t explicit_default_size = 0;

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
            case Value::BOOLEAN:
                return std::hash<bool>{}(value.boolean_value());
            case Value::FLOAT:
                return std::hash<double>{}(value.float_value());
            case Value::SIGNED:
                return std::hash<::int64_t>{}(value.signed_value());
            case Value::UNSIGNED:
                return std::hash<::uint64_t>{}(value.unsigned_value());
            case Value::VOID:
                return 0;
        }
    }
};

std::optional<Value> operator+(const std::optional<Value> a, const std::optional<Value> b);
std::optional<Value> operator-(const std::optional<Value> a, const std::optional<Value> b);
std::optional<Value> operator*(const std::optional<Value> a, const std::optional<Value> b);
std::optional<Value> operator/(const std::optional<Value> a, const std::optional<Value> b);
std::optional<Value> operator&&(const std::optional<Value> a, const std::optional<Value> b);
std::optional<Value> operator||(const std::optional<Value> a, const std::optional<Value> b);


bool operator>(const std::optional<Value> a, const std::optional<Value> b);
bool operator<(const std::optional<Value> a, const std::optional<Value> b);
bool operator>=(const std::optional<Value> a, const std::optional<Value> b);
bool operator<=(const std::optional<Value> a, const std::optional<Value> b);

#endif // VALUE_H
