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
    explicit Value(uint64_t value): type_(UNSIGNED), unsigned_value_(value) {}
    explicit Value(int64_t value);
    explicit Value(bool value): type_(BOOLEAN), boolean_value_(value) {}
    explicit Value(double value): type_(FLOAT), float_value_(value) {}

    [[nodiscard]] Type type() const {return type_;}
    [[nodiscard]] std::string type_name() const;
    [[nodiscard]] bool boolean_value() const;
    [[nodiscard]] double float_value() const;
    [[nodiscard]] int64_t signed_value() const;
    [[nodiscard]] uint64_t unsigned_value() const;

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator|(const Value& other) const;
    Value operator&(const Value& other) const;
    Value operator^(const Value& other) const;
    //Value operator%(const Value& other) const;
    Value operator<<(const Value& other) const;
    Value operator>>(const Value& other) const;
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

    [[nodiscard]] static uint64_t negate_signed(int64_t value);
    [[nodiscard]] static int64_t negate_unsigned(uint64_t value);
    [[nodiscard]] static uint64_t add_unsigned(uint64_t a, uint64_t b);
    [[nodiscard]] static int64_t add_signed(int64_t a, int64_t b);
    [[nodiscard]] static uint64_t multiply_unsigned(uint64_t a, uint64_t b);
    [[nodiscard]] static int64_t shift_left_signed(int64_t a, uint64_t b);
    [[nodiscard]] static uint64_t shift_left_unsigned(uint64_t a, uint64_t b);
};

#endif // VALUE_H
