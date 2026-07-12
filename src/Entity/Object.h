#ifdef IN_XLR8_OBJECT_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_OBJECT_H
#ifndef HAD_XLR8_OBJECT_H
#define HAD_XLR8_OBJECT_H

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

#include <set>

#include "Address.h"
#include "Body/Body.h"
#include "Entity/Entity.h"
#include "MemoryMap.h"
#include "StructuredValue.h"
#include "Token.h"
#include "Visibility.h"

class ObjectFile;

/**
  * @brief Represents an object.
  */
class Object: public Entity {
public:
    Object(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition);
    Object(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> parent_scope, bool default_only, const MemoryMap::Section* section);

    bool static less_pointers(const Object* a, const Object* b) {return *a < *b;}

    [[nodiscard]] bool is_reservation() const {return reservation_expression.has_value();}
    [[nodiscard]] bool empty() const {return !is_reservation() && body.empty();}
    [[nodiscard]] bool has_address() const {return address.has_value();}
    [[nodiscard]] std::optional<uint64_t> reservation() const;
    [[nodiscard]] SizeRange size_range() const;
    void uses(Symbol name) {explicitly_used_objects.insert(name);}

    void serialize(std::ostream& stream) const;

    bool operator<(const Object& other) const;

    const MemoryMap::Section* section;
    uint64_t alignment = 0;
    std::optional<Expression> reservation_expression;
    std::optional<Address> address;
    std::set<Symbol> explicitly_used_objects;

    Body body;

  protected:
    void evaluate_inner(EvaluationContext &context) override;

  private:
    static const Token token_address;
    static const Token token_alignment;
    static const Token token_body;
    static const Token token_reserve;
    static const Token token_section;
    static const Token token_uses;
};

std::ostream& operator<<(std::ostream& stream, const Object& node);

#endif // HAD_XLR8_OBJECT_H
#undef IN_XLR8_OBJECT_H
