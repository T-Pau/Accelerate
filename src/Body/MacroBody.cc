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

#include "Body/MacroBody.h"

#include "Body/Body.h"
#include "Body/ScopeBody.h"

void MacroBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << name;
    auto first = true;
    for (auto& argument : arguments) {
        if (first) {
            first = false;
        }
        else {
            stream << ",";
        }
        stream << " " << argument;
    }
    stream << std::endl;
}

void MacroBody::traverse(std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) {
    for (auto& argument : arguments) {
        expression_callback(argument);
    }
}

std::optional<Body> MacroBody::expand_calls() {
    for (auto& argument : arguments) {
        argument.expand_calls();
    }

    if (!macro) {
        throw LocationException(location, "internal error: macro {} not found", name);
    }

    if (arguments.empty()) {
        return macro->body.clone(CloneContext{});
    }

#if 0
    // create a new ScopeBody, add argument constants to it, map macro's argument constants to the new constants, and then clone the macro's body into it
    // TODO: we need the containing scope
    auto 
    auto new_body = ScopeBody::create(std::make_shared<Scope>(Visibility::ARGUMENT), Body{});
    auto scope_body = new_body.as<ScopeBody>();

    macro->set_arguments(scope_body, arguments);
    scope_body->append(macro->body.clone());
    macro->clear_arguments();
    return new_body;
#else
    throw LocationException(location, "expanding macro calls with arguments not yet implemented");
#endif
}

void MacroBody::resolve(Scope* scope, Entity* containing_entity) {
    BodyElement::resolve(scope, containing_entity);
    auto macro_ptr = scope->get_macro(name, containing_entity);
    if (!macro_ptr) {
        throw LocationException(location, "macro {} not found", name);
    }
    macro = macro_ptr.get();
}
