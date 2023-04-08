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

#include <iomanip>

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


bool Value::bolean_value() const {
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
    switch (type()) {
        case BOOLEAN:
        case VOID:
            throw Exception("can't add %s", type_name().c_str());

        case FLOAT:
            return Value(float_value_ + other.float_value());

        case SIGNED:
            switch (other.type()) {
                case BOOLEAN:
                case VOID:
                    throw Exception("can't add %s", other.type_name().c_str());

                case FLOAT:
                    return Value(float_value() + other.float_value_);

                case SIGNED: {
                    auto value = signed_value_ + other.signed_value();
                    if (value > signed_value_) {
                        throw Exception("result too small");
                    }
                    return Value(value);
                }

                case UNSIGNED:
                    // TODO: check for overflow
                    return Value(signed_value_ + other.unsigned_value());
            }
            break;

        case UNSIGNED:
            switch (other.type()) {
                case BOOLEAN:
                case VOID:
                    throw Exception("can't add %s", other.type_name().c_str());

                case FLOAT:
                    return Value(float_value() + other.float_value_);

                case SIGNED: {
                    // TODO: check for overflow
                    return Value(signed_value_ + other.unsigned_value());
                }

                case UNSIGNED: {
                    auto value = unsigned_value_ + other.unsigned_value();
                    if (value < unsigned_value_) {
                        throw Exception("result too large");
                    }
                    return Value(value);
                }
            }
            break;
    }
}
