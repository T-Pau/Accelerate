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

#include "Base.h"

#include <typeinfo>

std::string Base::type_name() const {
    auto raw_name = typeid(*this).name();

    auto p = raw_name;
    while (*p && !std::isalpha(*p) && *p != '_') {
        ++p;
    }
    return p;
}

std::ostream& operator<<(std::ostream& os, const Base& base) {
    base.serialize(os);
    return os;
}


#ifdef TRACE_TRANSLATION
#include <iostream>

#include <tpau-cpp-kernal/DiagnosticOutput.h>

using namespace tpau::cpp_kernal;

std::vector<const Base*> Base::trace_stack;
const char* Base::trace_type_names[] = {
    ">>>",
    "<<<",
    "---",
};

void Base::trace_implementation(TraceType type, std::string_view what, bool print_this, std::string_view message) const {
    if (type == TraceType::END) {
        trace_stack.pop_back();
    }

    std::cerr << std::string(trace_stack.size() * 2, ' ');
    std::cerr << trace_type_names[static_cast<int>(type)] << " " << type_name() << " " << this << " " << what;
    if (print_this) {
        std::cerr << " " << *this;
    }
    if (!message.empty()) {
        std::cerr << ": " << message;
    }
    std::cerr << std::endl;

    if (type == TraceType::BEGIN) {
        trace_stack.push_back(this);
    }
}

#endif
