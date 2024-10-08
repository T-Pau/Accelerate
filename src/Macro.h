/*
Macro.h --

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

#ifndef MACRO_H
#define MACRO_H

#include "Body.h"
#include "Callable.h"

class Macro: public Callable {
  public:
    Macro(ObjectFile* owner, const Token& name, const std::shared_ptr<ParsedValue>& definition);
    Macro(ObjectFile* owner, const Token& name, Visibility visibility, bool default_only, Callable::Arguments arguments, Body body);

    [[nodiscard]] Body expand(const std::vector<Expression>& arguments, std::shared_ptr<Environment> outer_environment) const;
    void serialize(std::ostream& stream) const;

    Body body;

  protected:
    [[nodiscard]] EvaluationContext evaluation_context(EvaluationResult& result) override;
    void evaluate_inner(EvaluationContext &context) override {body.evaluate(context);}

  private:
    static void initialize();

    static bool initialized;
    static Token token_body;
};

std::ostream& operator<<(std::ostream& stream, const Macro& macro);

#endif // MACRO_H
