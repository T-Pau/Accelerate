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

#include "UnnamedLabelList.h"

#include <tpau-cpp-kernal/LocationException.h>

using namespace tpau::cpp_kernal;

void UnnamedLabelList::add_label(const Location& location, Expression expression) {
    if (!labels.empty() && labels.back().location >= location) {
        throw LocationException(location, "internal error: unnamed labels must be added in order");
    }
    labels.emplace_back(location, std::move(expression));
}

std::optional<Expression> UnnamedLabelList::get_next_label(const Location& location) const {
    auto it = std::upper_bound(labels.begin(), labels.end(), Label(location, Expression()));
    if (it != labels.end()) {
        return it->expression;
    }
    else {
        return {};
    }
}

std::optional<Expression> UnnamedLabelList::get_previous_label(const Location& location) const {
    auto it = std::lower_bound(labels.rbegin(), labels.rend(), Label(location, Expression()));
    if (it != labels.rend()) {
        return it->expression;
    }
    else {
        return {};
    }
}
