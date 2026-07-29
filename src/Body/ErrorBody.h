#ifdef IN_XLR8_ERROR_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_ERROR_BODY_H
#ifndef HAD_XLR8_ERROR_BODY_H
#define HAD_XLR8_ERROR_BODY_H

/*
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

#include <tpau-cpp-kernal/LocationException.h>

#include "Body.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a body that reports an error when encoded. It can be used with conditional compilation to report errors when certain conditions are met: `.error message`.
 */
class ErrorBody : public BodyElement {
  public:
    static Body create(const Location& location, std::string message) { return Body(std::make_shared<ErrorBody>(location, std::move(message))); }

    explicit ErrorBody(const Location& location, std::string message) : location(location), message(std::move(message)) {}

    [[nodiscard]] Body clone(const CloneContext& context) const override { return Body(std::make_shared<ErrorBody>(location, message)); }

    [[nodiscard]] bool empty() const override { return false; }

    void encode(std::string& bytes, const Memory* memory) override { throw LocationException(location, message); }

    [[nodiscard]] std::optional<Body> evaluate(const EvaluationContext& context) override;
    void serialize(std::ostream& stream, const std::string& prefix) const override;

    Location location;
    std::string message;
};

#endif // HAD_XLR8_ERROR_BODY_H
#undef IN_XLR8_ERROR_BODY_H
