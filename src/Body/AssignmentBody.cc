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

#include "Body/AssignmentBody.h"

#include "Entity/Constant.h"
#include "Entity/ScopeEntity.h"

void AssignmentBody::enter_names(Scope* scope, Entity* containing_entity) {
    auto scope_entity = dynamic_cast<ScopeEntity*>(containing_entity);
    if (!scope_entity) {
        throw LocationException(location, "internal error: AssignmentBody: containing entity is not a ScopeEntity");
    }
    auto constant = std::make_shared<Constant>(location, name, visibility, scope_entity->scope(), false, value);
    scope_entity->add(constant);
}

void AssignmentBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix;
    if (visibility != Visibility::ARGUMENT) {
        stream << "." << visibility << " ";
    }
    stream << name << " = " << value << std::endl;
}
