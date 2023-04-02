/*
Expression.cc -- Abstract Base Class of Expression Nodes

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

#include "Expression.h"

#include <utility>
#include "ParseException.h"


std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Expression>& node) {
    node->serialize(stream);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const Expression& node) {
    node.serialize(stream);
    return stream;
}


std::shared_ptr<Expression> Expression::evaluate(std::shared_ptr<Expression> node, const Environment &environment) {
    auto new_node = node->evaluate(environment);

    if (!new_node) {
        return node;
    }
    else {
        return new_node;
    }
}



void Expression::set_byte_size(size_t size) {
    if (size != 0 && minimum_byte_size() != 0 && size < minimum_byte_size()) {
        throw ParseException(location, "value overflow");
    }
    byte_size_ = size;
}

void Expression::serialize(std::ostream &stream) const {
    serialize_sub(stream);
    if (byte_size() != 0 && (minimum_byte_size() == 0 || minimum_byte_size() != byte_size())) {
        stream << ":" << byte_size();
    }
}

std::vector<Symbol> Expression::get_variables() const {
    auto variables = std::vector<Symbol>();

    collect_variables(variables);

    return variables;
}


Expression::Iterator &Expression::Iterator::operator++() {
    while (!layers.empty()) {
        auto last = layers.back();
        last.current_child = last.node->iterate(last.current_child);
        if (last.current_child != nullptr) {
            layers.emplace_back(last.current_child);
            break;
        }
        else {
            layers.pop_back();
        }
    }
    return *this;
}
