/*
ExpressionList.h -- List of Expressions

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

#ifndef EXPRESSION_LIST_NODE_H
#define EXPRESSION_LIST_NODE_H

#include <vector>

#include "ExpressionNode.h"

class ExpressionList {
public:
    ExpressionList() = default;
    ExpressionList(std::initializer_list<std::shared_ptr<ExpressionNode>> list): expressions(list) {}

    [[nodiscard]] bool empty() const {return expressions.empty();}
    [[nodiscard]] size_t byte_size() const;
    void append(const ExpressionList& list) {expressions.insert(expressions.end(), list.expressions.begin(), list.expressions.end());}
    void append(const std::shared_ptr<ExpressionNode>& expression) {expressions.emplace_back(expression);}

    void serialize(std::ostream& stream) const;
    [[nodiscard]] std::vector<uint8_t> bytes(uint64_t byte_order) const;


    std::vector<std::shared_ptr<ExpressionNode>> expressions;

    [[nodiscard]] std::vector<std::shared_ptr<ExpressionNode>>::const_iterator begin() const {return expressions.begin();}
    [[nodiscard]] std::vector<std::shared_ptr<ExpressionNode>>::const_iterator end() const {return expressions.end();}
};

std::ostream& operator<<(std::ostream& stream, const ExpressionList& list);

#endif // EXPRESSION_LIST_NODE_H
