/*
UnnamedLabelList.cc --

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
#include "Exception.h"

size_t UnnamedLabelList::add_label(SizeRange offset) {
    entries.emplace_back(offset);
    return current_index();
}

size_t UnnamedLabelList::add_user() {
    entries.emplace_back();
    return current_index();
}

SizeRange UnnamedLabelList::get_next(size_t index) const {
    index += 1;
    while (index < entries.size()) {
        if (entries[index]) {
            return *entries[index];
        }
        index += 1;
    }

    throw Exception("no next unnamed label");
}

SizeRange UnnamedLabelList::get_previous(size_t index) const {
    while (index > 0) {
        index -= 1;
        if (entries[index]) {
            return *entries[index];
        }
    }

    throw Exception("no previous unnamed label");
}

void UnnamedLabelList::update_label(size_t index, SizeRange offset) {
    if (!entries[index]) {
        throw Exception("internal error: can't update unnamed label use");
    }
    entries[index] = offset;
}
