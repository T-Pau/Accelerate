#ifndef ENCODING_H
#define ENCODING_H

/*
Encoding.h --

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

#include <variant>

#include "BinaryEncoding.h"
#include "IntegerEncoding.h"
#include "StringEncoding.h"

class Encoding {
  public:
    explicit Encoding(IntegerEncoding encoding): encoding{encoding} {}
    explicit Encoding(const StringEncoding* encoding): encoding{encoding} {}
    explicit Encoding(const Value& value);
    explicit Encoding(Tokenizer& tokenizer);

    [[nodiscard]] const BinaryEncoding& as_binary_encoding() const {return std::get<BinaryEncoding>(encoding);}
    [[nodiscard]] const IntegerEncoding& as_integer_encoding() const {return std::get<IntegerEncoding>(encoding);}
    [[nodiscard]] const StringEncoding* as_string_encoding() const {return std::get<const StringEncoding*>(encoding);}
    [[nodiscard]] bool is_binary_encoding() const {return std::holds_alternative<BinaryEncoding>(encoding);}
    [[nodiscard]] bool is_integer_encoding() const {return std::holds_alternative<IntegerEncoding>(encoding);}
    [[nodiscard]] bool is_string_encoding() const {return std::holds_alternative<const StringEncoding*>(encoding);}
    [[nodiscard]] bool operator==(const Encoding& other) const;
    [[nodiscard]] bool operator==(const IntegerEncoding& other) const {return is_integer_encoding() && as_integer_encoding() == other;}

    void encode(std::string& bytes, const Value& value) const;
    [[nodiscard]] size_t encoded_size(const Value& value) const;
    [[nodiscard]] bool fits(const Value& value) const;
    [[nodiscard]] bool is_natural_encoding(const Value& value) const;
    void serialize(std::ostream& stream) const;

  private:
    std::variant<IntegerEncoding,BinaryEncoding,const StringEncoding*> encoding;
};

std::ostream& operator<<(std::ostream& stream, const Encoding& encoding);

#endif // ENCODING_H
