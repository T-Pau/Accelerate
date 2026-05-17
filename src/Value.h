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

/// Class representing a value of any type.
class Value {
public:
    /// @brief The type of a value.
    enum Type {
        BINARY, ///< Binary data
        BOOLEAN, ///< Boolean value
        FLOAT, ///< Floating point number
        INTEGER, ///< Integer
        NUMBER, ///< Number (integer or floating point)
        SIGNED, ///< Signed integer
        STRING, ///< String value
        UNSIGNED, ///< Unsigned integer
        VOID ///< No value
    };

    /// @brief Create void value.
    explicit Value() = default;

    /**
     * Create a value from an unsigned integer.
     * 
     * @param value The unsigned integer value.
     * @param default_size Explicitly set the default size of the value. Otherwise the smallest size that can hold the value is used.
     */
    explicit Value(uint64_t value, uint64_t default_size = 0): value{value}, explicit_default_size{default_size} {}

    /**
     * Create a value from a signed integer.
     * 
     * @param value The signed integer value.
     * @param default_size Explicitly set the default size of the value. Otherwise the smallest size that can hold the value is used.
     */
    explicit Value(int64_t value, uint64_t default_size = 0);

    /**
     * Create a value from a boolean.
     * 
     * @param value The boolean value.
     */
    explicit Value(bool value): value{value} {}

    /**
     * Create a value from a floating point number.
     * 
     * @param value The floating point value.
     */
    explicit Value(double value): value{value} {}

    /**
     * Create a value from binary data.
     * 
     * @param value The binary data value.
     */
    explicit Value(const std::string& value): value{value} {}

    /**
     * Create a value from a symbol.
     * 
     * @param value The symbol value.
     */
    explicit Value(Symbol value): value{value} {}

    /**
     * Get the name of a value type.
     * 
     * @param type The type to get the name of.
     * @return The name of the type.
     */
    static std::string type_name(Type type);

    /**
     * Check if value is binary data.
     * 
     * @return `true` if it is binary data, `false` otherwise.
     */
    [[nodiscard]] bool is_binary() const {return value && std::holds_alternative<std::string>(*value);}

    /**
     * Check if it is a boolean value.
     * 
     * @return `true` if it is a boolean value, `false` otherwise.
     */
    [[nodiscard]] bool is_boolean() const {return value && std::holds_alternative<bool>(*value);}

    /**
     * Check if it is a floating point number.
     * 
     * @return `true` if it is a floating point number, `false` otherwise.
     */
    [[nodiscard]] bool is_float() const {return value && std::holds_alternative<double>(*value);}

    /**
     * Check if it is an integer.
     * 
     * @return `true` if it is an integer, `false` otherwise.
     */
    [[nodiscard]] bool is_integer() const {return is_signed() || is_unsigned();}

    /**
     * Check if it is a number (integer or floating point).
     * 
     * @return `true` if it is a number, `false` otherwise.
     */
    [[nodiscard]] bool is_number() const {return is_integer() || is_float();}

    /**
     * Check if it is a signed integer.
     * 
     * @return `true` if it is a signed integer, `false` otherwise.
     */
    [[nodiscard]] bool is_signed() const {return value && std::holds_alternative<int64_t>(*value);}

    /**
     * Check if it is a string.
     * 
     * @return `true` if it is a string, `false` otherwise.
     */
    [[nodiscard]] bool is_string() const {return value && std::holds_alternative<Symbol>(*value);}

    /**
     * Check if it is an unsigned integer.
     * 
     * @return `true` if it is an unsigned integer, `false` otherwise.
     */
    [[nodiscard]] bool is_unsigned() const {return value && std::holds_alternative<uint64_t>(*value);}

    /**
     * Check if it is void (has no value).
     * 
     * @return `true` if it is void, `false` otherwise.
     */
    [[nodiscard]] bool is_void() const {return !value;}

    /**
     * Get the type of the value.
     * 
     * @return The type of the value.
     */
    [[nodiscard]] Type type() const;

    /**
     * Get the name of the value's type.
     * 
     * @return The name of the value's type.
     */
    [[nodiscard]] std::string type_name() const {return type_name(type());}

    /**
     * Get the binary data of the value.
     * 
     * @return The value as binary data.
     * @throws Exception if the value is not binary data.
     */
    [[nodiscard]] std::string binary_value() const;

    /**
     * Interpret value as a boolean.
     * 
     * Numbers are `true` if they are nonzero, strings are `true` if they are nonempty. Binary data and void values can't be interpreted as booleans.
     * 
     * @return The value as a boolean.
     * @throws Exception Value can't be interpreted as a boolean.
     */
    [[nodiscard]] bool boolean_value() const;

    /**
     * Get the floating point value.
     * 
     * @return The value as a floating point number.
     * @throws Exception if the value is not a number.
     */
    [[nodiscard]] double float_value() const;

    /**
     * Get the signed integer value.
     * 
     * @return The value as a signed integer.
     * @throws Exception if the value is not an integer or doesn't fit in a signed integer.
     */
    [[nodiscard]] int64_t signed_value() const;

    /**
     * Get string representation of the value.
     * 
     * Numbers are converted to their string representation, strings are returned as they are. Boolean, binary data and void values can't be interpreted as strings.
     * 
     * @return The value as a string.
     * @throws Exception if the value can't be interpreted as a string.
     */
    [[nodiscard]] std::string string_value() const;

    /**
     * Get the symbol for string value.
     * 
     * @return The value as a symbol.
     * @throws Exception if the value is not a string.
     */
    [[nodiscard]] Symbol symbol_value() const;

    /**
     * Get the unsigned integer value.
     * 
     * @return The value as an unsigned integer.
     * @throws Exception if the value is not an integer or doesn't fit in an unsigned integer.
      */
    [[nodiscard]] uint64_t unsigned_value() const;

    /**
     * Get the default size of the value.
     * 
     * For integer values, this is the smallest size that can hold the value, or the explicitly set default size if it is larger. For binary data, this is its size in bytes.
     * 
     * @return The default size of the value, if it has a default size.
     */
    [[nodiscard]] std::optional<uint64_t> default_size() const;

    /**
    * Interpret the value as a boolean.
    * 
    * Numbers are `true` if they are nonzero, strings are `true` if they are nonempty. Binary data and void values can't be interpreted as booleans.
    * 
    * @return The value as a boolean.
    * @throws Exception Value can't be interpreted as a boolean.
    */
    explicit operator bool() const {return boolean_value();}

    /**
    * Check if two values are equal.
    * 
    * @param other The value to compare with.
    * @return `true` if the values are equal, `false` otherwise.
    */
    bool operator==(const Value& other) const;

    /**
     * Check if two values are different.
     * 
     * @param other The value to compare with.
     * @return `true` if the values are different, `false` otherwise.
     */
    bool operator!=(const Value& other) const {return !(*this == other);}

    /**
     * Check if this value is less than another value. Only numbers are comparable.
     * 
     * @param other The value to compare with.
     * @return `true` if both values are numbers and this value is less than the other value, `false` otherwise.
     */
    bool operator<(const Value& other) const;

    /**
     * Check if this value is greater than another value. Only numbers are comparable.
     * 
     * @param other The value to compare with.
     * @return `true` if both values are numbers and this value is greater than the other value, `false` otherwise.
     */
    bool operator>(const Value& other) const {return other < *this;}
    
    /**
     * Check if this value is less than or equal to another value. Only numbers are comparable.
     * 
     * @param other The value to compare with.
     * @return `true` if both values are numbers and this value is less than or equal to the other value, `false` otherwise.
     */
    bool operator<=(const Value& other) const;

    /**
     * Check if this value is greater than or equal to another value. Only numbers are comparable.
     * 
     * @param other The value to compare with.
     * @return `true` if both values are numbers and this value is greater than or equal to the other value, `false` otherwise.
     */
    bool operator>=(const Value& other) const {return other <= *this;}

    /**
     * Negate value.
     * 
     * @return The negated value.
     * @throws Exception if the value is not a number.
     */
    Value operator-() const;

    /**
     * Bitwise NOT of the value.
     * 
     * @return The bitwise NOT of the value.
     * @throws Exception if the value is not an integer.
     */
    Value operator~() const;

    /**
     * Add another value to this value.
     * 
     * @param other The value to add.
     * @return The sum of the two values.
     * @throws Exception if either value is not a number or if the addition would overflow.
     */
    Value operator+(const Value& other) const;

    /**
     * Subtract another value from this value.
     * 
     * @param other The value to subtract.
     * @return The difference of the two values.
     * @throws Exception if either value is not a number or if the subtraction would overflow.
     */
    Value operator-(const Value& other) const;

    /**
     * Divide this value by another value.
     * 
     * @param other The value to divide by.
     * @return The quotient of the two values.
     * @throws Exception if either value is not a number or if dividing by zero.
     */
    Value operator/(const Value& other) const;

    /**
     * Multiply this value by another value.
     * 
     * @param other The value to multiply by.
     * @return The product of the two values.
     * @throws Exception if either value is not a number or if the multiplication would overflow.
     */
    Value operator*(const Value& other) const;

    /**
     * Bitwise OR of this value and another value.
     * 
     * @param other The value to OR with.
     * @return The result of the bitwise OR operation.
     * @throws Exception if either value is not an unsigned integer.
     */
    Value operator|(const Value& other) const;

    /**
     * Bitwise AND of this value and another value.
     * 
     * @param other The value to AND with.
     * @return The result of the bitwise AND operation.
     * @throws Exception if either value is not an unsigned integer.
     */
    Value operator&(const Value& other) const;

    /**
     * Bitwise AND of this value and an unsigned integer.
     * 
     * @param other The unsigned integer to AND with.
     * @return The result of the bitwise AND operation.
     * @throws Exception if this value is not an unsigned integer.
     */
    Value operator&(uint64_t other) const {return *this & Value(other);}

    /**
     * Bitwise exclusive OR of this value and another value.
     * 
     * @param other The value to exclusive OR with.
     * @return The result of the bitwise exclusive OR operation.
     * @throws Exception if either value is not an unsigned integer.
     */
    Value operator^(const Value& other) const;

    /**
     * The remainder of this value divided by another value.
     * 
     * @param other The value to divide by.
     * @return The remainder of the division.
     * @throws Exception if either value is not an unsigned integer or if dividing by zero.
     */
    Value operator%(const Value& other) const;

    /**
    * Left shift this value by another value.
    * 
    * @param other The value to shift by.
    * @return The result of the left shift operation.
    * @throws Exception if either value is not an integer or an overflow occurs.
    */
    Value operator<<(const Value& other) const;

    /**
     * Right shift this value by another value.
     * 
     * @param other The value to shift by.
     * @return The result of the right shift operation.
     * @throws Exception if either value is not an integer or an overflow occurs.
     */
    Value operator>>(const Value& other) const;

    /**
     * Left shift this value by an unsigned integer.
     * 
     * @param other The unsigned integer to shift by.
     * @return The result of the left shift operation.
     * @throws Exception if this value is not an integer or an overflow occurs.
     */
    Value operator>>(uint64_t other) const {return *this >> Value(other);}

    /**
     * Logical AND of this value and another value.
     * 
     * @param other The value to AND with.
     * @return True if both values are true.
     * @throws Exception if either value can't be interpreted as a boolean.
     */
    Value operator&&(const Value& other) const;
    /**
     * Logical OR of this value and another value.
     * 
     * @param other The value to OR with.
     * @return True if either value is true.
     * @throws Exception if either value can't be interpreted as a boolean.
     */
    Value operator||(const Value& other) const;

    /**
     * Add another value to this value, modifying this value.
      * 
      * @param other The value to add.
      * @return This value after addition.
      * @throws Exception if either value is not a number or if the addition would overflow.
     */
    Value& operator+=(const Value& other) {*this = *this + other; return *this;}

    /**
     * Subtract another value from this value, modifying this value.
      * 
      * @param other The value to subtract.
      * @return This value after subtraction.
      * @throws Exception if either value is not a number or if the subtraction would overflow.
     */
    Value& operator-=(const Value& other) {*this = *this - other; return *this;}

    /** 
     * Divide this value by another value, modifying this value.
      * 
      * @param other The value to divide by.
      * @return This value after division.
      * @throws Exception if either value is not a number or if dividing by zero.
     */
    Value& operator/=(const Value& other) {*this = *this / other; return *this;}

    /**
     * Multiply this value by another value, modifying this value.
      * 
      * @param other The value to multiply by.
      * @return This value after multiplication.
      * @throws Exception if either value is not a number or if the multiplication would overflow.
     */
    Value& operator*=(const Value& other) {*this = *this * other; return *this;}

    /**
     * Serialize the value to a stream.
     * 
     * @param stream The stream to serialize to.
     */
    void serialize(std::ostream& stream) const;

private:
    /// @brief The value, or `std::nullopt` if it is void.
    std::optional<std::variant<bool, double, int64_t, uint64_t, Symbol, std::string>> value;

    /// @brief The explicitly set default size of the value, or 0 if the default size is not explicitly set.
    uint64_t explicit_default_size{0};

    /**
     * Get the binary data of the value without conversions.
     * 
     * @return The value as binary data.
     * @throws std::bad_variant_access if the value is not binary data.
     */
    [[nodiscard]] auto raw_binary_value() const {return std::get<std::string>(*value);}

    /**
     * Get the boolean value without conversions.
     * 
     * @return The value as a boolean.
     * @throws std::bad_variant_access if the value is not a boolean.
     */
    [[nodiscard]] auto raw_boolean_value() const {return std::get<bool>(*value);}

    /**
     * Get the floating point value without conversions.
     * 
     * @return The value as a floating point number.
     * @throws std::bad_variant_access if the value is not a floating point number.
     */
    [[nodiscard]] auto raw_float_value() const {return std::get<double>(*value);}

    /**
     * Get the signed integer value without conversions.
     * 
     * @return The value as a signed integer.
     * @throws std::bad_variant_access if the value is not a signed integer.
     */
    [[nodiscard]] auto raw_signed_value() const {return std::get<int64_t>(*value);}

    /** 
     * Get the string value without conversions.
     * 
     * @return The value as a symbol.
     * @throws std::bad_variant_access if the value is not a string.
     */
    [[nodiscard]] auto raw_symbol_value() const {return std::get<Symbol>(*value);}

    /**
     * Get the unsigned integer value without conversions.
     * 
     * @return The value as an unsigned integer.
     * @throws std::bad_variant_access if the value is not an unsigned integer.
     */
    [[nodiscard]] auto raw_unsigned_value() const {return std::get<uint64_t>(*value);}

    /**
     * Negate a signed integer.
     * 
     * @param value The value to negate.
     * @return The negated value.
     */
    [[nodiscard]] static uint64_t negate_signed(int64_t value);

    /**
     * Negate an unsigned integer, checking for overflow.
     * 
     * @param value The value to negate.
     * @return The negated value.
     * @throws Exception if an overflow occurs.
     */
    [[nodiscard]] static int64_t negate_unsigned(uint64_t value);

    /**
     * Add two unsigned integers, checking for overflow.
     * 
     * @param a The first integer.
     * @param b The second integer.
     * @return The sum of the two integers.
     * @throws Exception if an overflow occurs.
     */
    [[nodiscard]] static uint64_t add_unsigned(uint64_t a, uint64_t b);

    /**
     * Add two signed integers, checking for overflow.
     * 
     * @param a The first integer.
     * @param b The second integer.
     * @return The sum of the two integers.
     * @throws Exception if an overflow occurs.
     */
    [[nodiscard]] static int64_t add_signed(int64_t a, int64_t b);

    /**
     * Multiply two unsigned integers, checking for overflow.
     * 
     * @param a The first integer.
     * @param b The second integer.
     * @return The product of the two integers.
     * @throws Exception if an overflow occurs.
     */
    [[nodiscard]] static uint64_t multiply_unsigned(uint64_t a, uint64_t b);

    /**
     * Shift a signed integer to the left, checking for overflow.
     * 
     * @param a The integer to shift.
     * @param b The number of bits to shift.
     * @return The shifted integer.
     * @throws Exception if an overflow occurs.
     */
    [[nodiscard]] static int64_t shift_left_signed(int64_t a, uint64_t b);

    /**
     * Shift an unsigned integer to the left, checking for overflow.
     * 
     * @param a The integer to shift.
     * @param b The number of bits to shift.
     * @return The shifted integer.
     * @throws Exception if an overflow occurs.
     */
    [[nodiscard]] static uint64_t shift_left_unsigned(uint64_t a, uint64_t b);
};

/**
 * Output a value to a stream.
 * 
 * @param stream The stream to output to.
 * @param value The value to output.
 * @return The stream after outputting the value.
 */
std::ostream& operator<<(std::ostream& stream, const Value& value);
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

/**
 * Add two optional values. If either value is not set, the result is not set.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return The sum of the two values, or not set if either value is not set.
 * @throws Exception if either value is not a number or if the addition would overflow.
 */
std::optional<Value> operator+(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Subtract two optional values. If either value is not set, the result is not set.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return The difference of the two values, or not set if either value is not set.
 * @throws Exception if either value is not a number or if the subtraction would overflow.
 */
std::optional<Value> operator-(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Multiply two optional values. If either value is not set, the result is not set.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return The product of the two values, or not set if either value is not set.
 * @throws Exception if either value is not a number or if the multiplication would overflow.
 */
std::optional<Value> operator*(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Divide two optional values. If either value is not set, the result is not set.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return The quotient of the two values, or not set if either value is not set.
 * @throws Exception if both values are set and either value is not a number, an overflow occurs, or if dividing by zero.
 */
std::optional<Value> operator/(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Logical AND of two optional values. If either value is not set, the result is not set.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return The result of the logical AND operation, or not set if either value is not set.
 * @throws Exception if both values are set and either value can't be interpreted as a boolean
 */
std::optional<Value> operator&&(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Logical OR of two optional values. If either value is not set, the result is not set.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return The result of the logical OR operation, or not set if either value is not set.
 * @throws Exception if both values are set and either value can't be interpreted as a boolean
 */
std::optional<Value> operator||(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Check if a value is greater than another value. If either value is not set, the result is `false`.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return `true` if both values are set and the first value is greater than the second value, `false` otherwise.
 * @throws Exception if both values are set and they are not comparable.
 */
bool operator>(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Check if a value is less than another value. If either value is not set, the result is `false`.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return `true` if both values are set and the first value is less than the second value, `false` otherwise.
 * @throws Exception if both values are set and they are not comparable.
 */
bool operator<(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Check if a value is greater than or equal to another value. If either value is not set, the result is `false`.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return `true` if both values are set and the first value is greater than or equal to the second value, `false` otherwise.
 * @throws Exception if both values are set and they are not comparable.
 */
bool operator>=(const std::optional<Value>& a, const std::optional<Value>& b);

/**
 * Check if a value is less than or equal to another value. If either value is not set, the result is `false`.
 * 
 * @param a The first value.
 * @param b The second value.
 * @return `true` if both values are set and the first value is less than or equal to the second value, `false` otherwise.
 * @throws Exception if both values are set and they are not comparable.
 */
bool operator<=(const std::optional<Value>& a, const std::optional<Value>& b);

#endif // VALUE_H
