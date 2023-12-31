/*
ScopeBody.cc --

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

#include "ScopeBody.h"

void ScopeBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".scope {" << std::endl;
    body.serialize(stream, prefix + "  ");
    stream << prefix << "}" << std::endl;
}

std::optional<Body> ScopeBody::evaluated(const EvaluationContext& context) const {
    // TODO: implement properly to isolate unnamed labels
    auto new_body = body.evaluated(context);
    if (new_body) {
        return new_body->scoped();
    }
    else {
        return {};
    }
}

Body ScopeBody::create(Body body) {
    if (body.empty()) {
        return body;
    }
    else {
        return Body(std::make_shared<ScopeBody>(std::move(body)));
    }
}
