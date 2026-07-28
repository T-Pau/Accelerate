#ifdef IN_XLR8_MACRO_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_MACRO_H
#ifndef HAD_XLR8_MACRO_H
#define HAD_XLR8_MACRO_H

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

#include "Body/Body.h"
#include "CallableArguments.h"
#include "Entity/ScopeEntity.h"

/**
 * @brief Represents a macro.
 */
class Macro : public ScopeEntity {
  public:
    Macro(const Location& location, Symbol name, std::shared_ptr<Scope> containing_scope, const std::shared_ptr<StructuredValue>& definition);

    Macro(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> containing_scope, bool default_only, CallableArguments arguments);

    [[nodiscard]] Body expand(const std::vector<Expression>& arguments, std::shared_ptr<Scope> outer_environment) const;
    void serialize(std::ostream& stream) const override;

    void enter_names() { body.enter_names(scope().get(), this); }

    template <ScopeBodyOrExpression Expansion> void set_arguments(Expansion* expansion, const std::vector<Expression>& arguments) { this->arguments.set_arguments(expansion, arguments); }

    void clear_arguments() { this->arguments.clear_arguments(); }

    Body body;
    CallableArguments arguments;

  protected:
    [[nodiscard]] EvaluationContext evaluation_context(EvaluationResult& result) override;

    void traverse(std::function<void(Entity&)> entity_callback, std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) override;

    void resolve_implementation() override;

  private:
    static void initialize();

    static bool initialized;
    static Token token_body;
};

std::ostream& operator<<(std::ostream& stream, const Macro& macro);

#endif // HAD_XLR8_MACRO_H
#undef IN_XLR8_MACRO_H
