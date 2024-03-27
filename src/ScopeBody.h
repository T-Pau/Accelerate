/*
ScopeBody.h --

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

#ifndef SCOPE_BODY_H
#define SCOPE_BODY_H

#include "Body.h"
#include "Exception.h"

class ScopeBody: public BodyElement {
  public:
    static Body create(Body body, const std::shared_ptr<Environment>& inner_environment);
    explicit ScopeBody(Body body_, const std::shared_ptr<Environment>& inner_environment): body(std::move(body_)), environment(inner_environment) {size_range_=body.size_range();}

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone ScopeBody");}
    [[nodiscard]] bool empty() const override {return body.empty();}
    void encode(std::string &bytes, const Memory *memory) const override {body.encode(bytes, memory);}
    [[nodiscard]] std::optional<Body> evaluated(const EvaluationContext &context) const override;
    void serialize(std::ostream &stream, const std::string &prefix) const override;

  private:
    Body body;
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
};


#endif // SCOPE_BODY_H
