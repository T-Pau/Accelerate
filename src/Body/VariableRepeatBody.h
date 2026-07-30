#ifdef IN_XLR8_VARIABLE_REPEAT_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_VARIABLE_REPEAT_BODY_H
#ifndef HAD_XLR8_VARIABLE_REPEAT_BODY_H
#define HAD_XLR8_VARIABLE_REPEAT_BODY_H

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

#include <tpau-cpp-kernal/Exception.h>

#include "Body/RepeatBody.h"
#include "Scope.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a body that repeats a block of code a specified number of times using a counter variable.
 */
class VariableRepeatBody : public RepeatBody {
  public:
    VariableRepeatBody(std::shared_ptr<Scope> containing_scope, Location variable_location, Symbol variable, RepeatRange range, Body body);

    static Body create(std::shared_ptr<Scope> containing_scope, Location variable_location, Symbol variable, Expression start, Expression end, Body body) { return create(std::move(containing_scope), variable_location, variable, RepeatRange(std::move(start), std::move(end)), std::move(body)); }

    static Body create(std::shared_ptr<Scope> containing_scope, Location variable_location, Symbol variable, RepeatRange range, Body body);

    [[nodiscard]] Body clone(const CloneContext& context) const override;

    void encode(std::string& bytes, const Memory* memory) override { throw Exception("can't encode unresolved repeat"); }

    [[nodiscard]] std::optional<Body> evaluate(const EvaluationContext& context) override;
    void serialize(std::ostream& stream, const std::string& prefix) const override;
    void resolve(Scope* scope, Entity* containing_entity) override;

  private:
    Body expand();

    Symbol variable;
    Location variable_location;
    std::shared_ptr<Constant> variable_constant;
    std::shared_ptr<Scope> inner_scope;
};

#endif // HAD_XLR8_VARIABLE_REPEAT_BODY_H
#undef IN_XLR8_VARIABLE_REPEAT_BODY_H
