#ifdef IN_XLR8_OUTPUT_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_OUTPUT_H
#ifndef HAD_XLR8_OUTPUT_H
#define HAD_XLR8_OUTPUT_H

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
#include "Entity/ScopeEntity.h"

class Output : public ScopeEntity {
  public:
    Output(const Location& location, const Target* target, Body body);

    [[nodiscard]] const Body& get_body() const { return body; }

    void enter_names() { body.enter_names(scope().get(), this); }

    void resolve_implementation() override { body.resolve(scope().get(), this); }

    // TODO: implement
    // void serialize(std::ostream& stream) const override;

    Body body;

  protected:
    void evaluate_inner(EvaluationContext& context) override {}; // TODO: implement

  private:
    static Token token_output;
};

#endif // HAD_XLR8_OUTPUT_H
#undef IN_XLR8_OUTPUT_H
