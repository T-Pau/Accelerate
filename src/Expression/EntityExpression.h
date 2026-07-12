#ifndef HAD_XLR8_ENTITY_EXPRESSION_H
#define HAD_XLR8_ENTITY_EXPRESSION_H

/*
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

#include "Expression/BaseExpression.h"
#include "Entity/Entity.h"

/**
 * @brief Represents an expression referring to a constant or an object.
 */
class EntityExpression: public BaseExpression {
public:
    explicit EntityExpression(const Location& location, const Entity* entity): BaseExpression(location), entity(entity) {}

protected:
    [[nodiscard]] std::optional<Expression> evaluate(const EvaluationContext& context) override;
    void resolve(Scope* scope, Entity* containing_entity) override;

    void serialize_sub(std::ostream& stream) const override {stream << entity->name;}

protected:
    friend class Expression;
    
    const Entity* entity;
};

#endif // HAD_XLR8_ENTITY_EXPRESSION_H
