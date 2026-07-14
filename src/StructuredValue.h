#ifdef IN_XLR8_STRUCTURED_VALUE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_STRUCTURED_VALUE_H
#ifndef HAD_XLR8_STRUCTURED_VALUE_H
#define HAD_XLR8_STRUCTURED_VALUE_H

/*
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


#include "Token.h"
#include "FileTokenizer.h"
#include "TokenGroup.h"

class StructuredArray;
class StructuredBody;
class StructuredDictionary;
class StructuredScalar;

/**
 * @brief Represents a structured value, which can be an scalar, array, dictionary, or body.
 */
class StructuredValue {
public:
    enum Type {
        ARRAY,
        BODY,
        DICTIONARY,
        SCALAR_LIST,
        SCALAR_SINGULAR
    };

    virtual ~StructuredValue() = default;

    [[nodiscard]] virtual Type type() const = 0;

    static void setup(FileTokenizer& tokenizer);
    static std::shared_ptr<StructuredValue> parse(Tokenizer& tokenizer);

    template <typename T>
    [[nodiscard]] T* as() {return dynamic_cast<T*>(this);}
    template <typename T>
    [[nodiscard]] const T* as() const {return dynamic_cast<const T*>(this);}
    template <typename T>
    [[nodiscard]] bool is() const {return as<T>() != nullptr;}

    [[nodiscard]] const StructuredArray* as_array() const;
    [[nodiscard]] const StructuredBody* as_body() const;
    [[nodiscard]] const StructuredDictionary* as_dictionary() const;
    [[nodiscard]] const StructuredScalar* as_scalar() const;
    [[nodiscard]] const StructuredScalar* as_singular_scalar() const;
    [[nodiscard]] bool is_array() const {return type() == ARRAY;}
    [[nodiscard]] bool is_body() const {return type() == BODY;}
    [[nodiscard]] bool is_dictionary() const {return type() == DICTIONARY;}
    [[nodiscard]] bool is_scalar() const {return type() == SCALAR_SINGULAR || type() == SCALAR_LIST;}
    [[nodiscard]] bool is_singular_scalar() const {return type() == SCALAR_SINGULAR;}

    Location location;

protected:
    static void initialize();
    static bool initialized;
    static TokenGroup start_group;
};


#endif // HAD_XLR8_STRUCTURED_VALUE_H
#undef IN_XLR8_STRUCTURED_VALUE_H
