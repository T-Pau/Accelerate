/*
MacroBody.cc --

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

#include "MacroBody.h"

#include "Body.h"
#include "Macro.h"

std::optional<Body> MacroBody::evaluated(const EvaluationContext& context) const {
    std::vector<Expression> new_arguments;
    auto changed = false;
    for (auto& argument: arguments) {
        auto new_argument = argument.evaluated(context);
        if (new_argument) {
            new_arguments.emplace_back(*new_argument);
            changed = true;
        }
        else {
            new_arguments.emplace_back(argument);
        }
    }

    if (auto macro = context.environment->get_macro(name.as_symbol())) {
        if (!context.conditional) {
            return macro->expand(new_arguments);
        }
    }
    else {
        context.result.add_unresolved_macro(name.as_symbol());
    }

    if (changed) {
        return Body(name, new_arguments);
    }
    else {
        return {};
    }
}


void MacroBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << name.as_string();
    auto first = true;
    for (auto& argument: arguments) {
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
