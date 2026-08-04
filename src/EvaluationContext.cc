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

#include "EvaluationContext.h"

#include <tpau-cpp-kernal/Exception.h>

#include "Entity/Entity.h"

using namespace tpau::cpp_kernal;

EvaluationContext::EvaluationContext(Entity* entity) : entity(entity), environment(entity->containing_scope()), offset(0) {}

EvaluationContext EvaluationContext::adding_offset(const SizeRange& size) const {
    auto new_context = *this;
    new_context.offset += size;
    return new_context;
}

EvaluationContext EvaluationContext::setting_offset(const SizeRange& new_offset) const {
    auto new_context = *this;
    new_context.offset = new_offset;
    return new_context;
}

EvaluationContext EvaluationContext::making_conditional() const {
    auto new_context = *this;
    new_context.conditional = true;
    return new_context;
}

EvaluationContext EvaluationContext::adding_scope(std::shared_ptr<Scope> new_environment) const {
    auto new_context = *this;
    new_context.environment = std::move(new_environment);
    return new_context;
}
