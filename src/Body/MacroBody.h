#ifdef IN_XLR8_MACRO_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_MACRO_BODY_H
#ifndef HAD_XLR8_MACRO_BODY_H
#define HAD_XLR8_MACRO_BODY_H

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

#include <tpau-cpp-kernal/LocationException.h>

#include "Body/BodyElement.h"
#include "Entity/Macro.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a macro call: `macro_name(arg1, arg2, ...)`.
 */
class MacroBody : public BodyElement {
  public:
    static Body create(const Location& location, Symbol name, std::vector<Expression> arguments, Macro* macro = {}) { return Body(std::make_shared<MacroBody>(location, name, std::move(arguments), macro)); }

    MacroBody(Location location, Symbol name, std::vector<Expression> arguments, Macro* macro = {}) : BodyElement(location, SizeRange(0, {})), name(name), macro(macro), arguments(std::move(arguments)) {}

    [[nodiscard]] Body clone(const CloneContext& context) const override { throw LocationException(location, "internal error: can't clone unexpanded macro call"); }

    void encode(std::string& bytes, const Memory* memory) override { throw LocationException(location, "can't encode unexpanded macro call"); }

    void serialize(std::ostream& stream, const std::string& prefix) const override;
    std::optional<Body> expand_calls() override;
    void traverse(std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) override;

    void resolve(Scope* scope, Entity* containing_entity) override;

    /// @brief The name of the macro being called.
    Symbol name;

    /// @brief The macro being called.
    Macro* macro{};

    /// @brief The arguments passed to the macro.
    std::vector<Expression> arguments;
};

#endif // HAD_XLR8_MACRO_BODY_H
#undef IN_XLR8_MACRO_BODY_H
