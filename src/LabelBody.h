/*
LabelBody.h --

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

#ifndef LABEL_BODY_H
#define LABEL_BODY_H

#include "Body.h"
#include "Exception.h"
#include "SizeRange.h"

class LabelBody: public BodyElement {
public:
    explicit LabelBody(Symbol name): name(name) {}
    LabelBody(Symbol name, const SizeRange& offset, bool added_to_environment, size_t unnamed_index): name(name), offset(offset), unnamed_index(unnamed_index), added_to_environment(added_to_environment) {}

    void encode(std::string &bytes, const Memory* memory) const override {}
    [[nodiscard]] bool empty() const override {return added_to_environment && offset.size();}
    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone label");}
    [[nodiscard]] std::optional<Body> evaluated(const EvaluationContext& context) const override;

    void serialize(std::ostream &stream, const std::string& prefix) const override;

    static Symbol unnamed_label_name(uint64_t number);

    Symbol name;
    SizeRange offset = SizeRange(0, {});
    size_t unnamed_index = std::numeric_limits<size_t>::max();
    bool added_to_environment = false;
};


#endif // LABEL_BODY_H
