#ifdef IN_XLR8_CONSTANT_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_CONSTANT_H
#ifndef HAD_XLR8_CONSTANT_H
#define HAD_XLR8_CONSTANT_H

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

#include "Entity/Entity.h"

class Constant : public Entity {
  public:
    Constant(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> containing_scope, bool default_only, Expression value) : Entity(location, name, visibility, containing_scope, default_only), value(std::move(value)) {}

    Constant(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition);

    [[nodiscard]] bool has_value() const { return value.has_value(); }

    void serialize(std::ostream& stream) const override;

    void add_reference() { reference_count += 1; }

    void remove_reference() {
        if (reference_count == 0) {
            throw LocationException(location, "removing reference from constant with zero references");
        }
        reference_count -= 1;
    }

    [[nodiscard]] bool is_referenced() const { return reference_count > 0; }

    [[nodiscard]] bool single_reference() const { return reference_count == 1; }

    Expression value;

  protected:
    void traverse(std::function<void(Entity&)> entity_callback, std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) override { expression_callback(value); }

  private:
    /**
     * @brief The number of ConstantExpressions that refer to this constant.
     *
     * It is used to determine when a ScopeBody can remove a constant from its scope and when an ArgumentConstant can be inlined.
     *
     * We don't use `std::shared_ptr`'s reference counting because we only keep a weak reference to the constant in ConstantExpression. Also, counting it ourselves is more precise.
     */
    size_t reference_count{};

    static const Token token_value;
};

#endif // HAD_XLR8_CONSTANT_H
#undef IN_XLR8_CONSTANT_H
