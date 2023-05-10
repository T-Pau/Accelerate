/*
BodyElement.cc --

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "BodyElement.h"
#include "BodyBlock.h"

std::ostream& operator<<(std::ostream& stream, const BodyElement& element) {
    element.serialize(stream, "");
    return stream;
}

std::shared_ptr<BodyElement> BodyElement::evaluate(std::shared_ptr<BodyElement> element, const Environment &environment) {
    auto result = element->evaluate(environment, 0, 0);
    if (result.element) {
        return result.element;
    }
    else {
        return element;
    }
}

std::shared_ptr<BodyElement> BodyElement::append(const std::shared_ptr<BodyElement>& body, const std::shared_ptr<BodyElement>& element) {
    if (element->empty()) {
        return body;
    }
    if (!body) {
        return element;
    }

    auto new_body = body->append_sub(body, element);
    if (new_body) {
        return new_body;
    }
    else {
        return std::make_shared<BodyBlock>(std::vector<std::shared_ptr<BodyElement>>({body, element}));
    }
}

std::shared_ptr<BodyElement> BodyElement::make_unique(std::shared_ptr<BodyElement> element) {
    if (element.use_count() < 2) {
        return element;
    }
    else {
        return element->clone();
    }
}

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BodyElement>& element) {
    stream << *element;
    return stream;
}
