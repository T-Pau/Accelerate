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

#include <cstddef>

#include "Node.h"
#include "FileTokenizer.h"
#include "Int.h"

class Object;
class Expression;
class Environment;

class BaseExpression {
public:
    enum Type {
        BINARY,
        FUNCTION,
        LABEL,
        OBJECT,
        UNARY,
        VALUE,
        VARIABLE,
        VOID
    };

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = BaseExpression*;
        using pointer = BaseExpression**;
        using reference = BaseExpression*&;

        Iterator() = default;
        explicit Iterator(BaseExpression*expression){ layers.emplace_back(expression);}

        reference operator*() { return layers.back().node;}
        pointer operator->() { return &layers.back().node;}

        Iterator& operator++();
        Iterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

        bool operator==(const Iterator& other) const {return layers == other.layers;}
        bool operator!=(const Iterator& other) const { return !(*this == other);}

    private:
        class Layer {
        public:
            explicit Layer(BaseExpression* node): node(node) {}
            bool operator==(const Layer& other) const {return node == other.node && current_child == other.current_child;}
            BaseExpression* node;
            BaseExpression* current_child = nullptr;
        };
        std::vector<Layer> layers;
    };

    Iterator begin() {return Iterator(this);}
    Iterator end() {return {};}

    [[nodiscard]] virtual Type type() const = 0;
    [[nodiscard]] virtual bool has_value() const {return value().has_value();}
    [[nodiscard]] virtual std::optional<Value> value() const {return {};}
    [[nodiscard]] virtual std::optional<Value> minimum_value() const {return value();}
    [[nodiscard]] virtual std::optional<Value> maximum_value() const {return value();}

    void serialize(std::ostream& stream) const;

    [[nodiscard]] virtual std::optional<Expression> evaluated(const Environment& environment) const = 0;
    virtual void collect_objects(std::unordered_set<Object*>& objects) const {}

    Location location;

protected:
    virtual BaseExpression* iterate(BaseExpression* last) const {return nullptr;}
    virtual void serialize_sub(std::ostream& stream) const = 0;
};

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<BaseExpression>& node);
std::ostream& operator<<(std::ostream& stream, const BaseExpression& node);

#endif // BASE_EXPRESSION_H
