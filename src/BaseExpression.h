/*
BaseExpression.h -- Abstract Base Class of BaseExpression Nodes

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

#ifndef BASE_EXPRESSION_H
#define BASE_EXPRESSION_H

#include "FileTokenizer.h"

class Object;
class Expression;
class EvaluationContext;
class Environment;

class BaseExpression {
public:
    BaseExpression() = default;
    explicit BaseExpression(Location location): location(location) {}
    virtual ~BaseExpression() = default;

    [[nodiscard]] virtual bool has_value() const {return value().has_value();}
    [[nodiscard]] virtual std::optional<Value> value() const {return {};}
    [[nodiscard]] virtual std::optional<Value> minimum_value() const {return value();}
    [[nodiscard]] virtual std::optional<Value> maximum_value() const {return value();}
    [[nodiscard]] virtual std::optional<Value::Type> type() const {return {};}

    void serialize(std::ostream& stream) const;

    [[nodiscard]] virtual std::optional<Expression> evaluated(const EvaluationContext& context) const = 0;
    virtual void collect_objects(std::unordered_set<Object*>& objects) const {}

    Location location;

protected:
    virtual void serialize_sub(std::ostream& stream) const = 0;
};

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BaseExpression>& node);
std::ostream& operator<<(std::ostream& stream, const BaseExpression& node);

#endif // BASE_EXPRESSION_H
