#ifdef IN_XLR8_ENTITY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_ENTITY_H
#ifndef HAD_XLR8_ENTITY_H
#define HAD_XLR8_ENTITY_H

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

#include "Base.h"
#include "EvaluationContext.h"
#include "StructuredValue.h"
#include "Unresolved.h"
#include "Visibility.h"

class Constant;
class Function;
class Macro;
class Object;
class Scope;

/**
 * @brief Represents an entity in the source code, which can be a constant, object, macro, or function.
 */
class Entity : public Base {
  public:
    Entity(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition);
    Entity(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> parent_scope, bool default_only = false);
    virtual ~Entity() = default;

    template <typename T> [[nodiscard]] T* as() { return dynamic_cast<T*>(this); }

    template <typename T> [[nodiscard]] const T* as() const { return dynamic_cast<const T*>(this); }

    template <typename T> [[nodiscard]] bool is() const { return as<T>() != nullptr; }

    [[nodiscard]] bool operator<(const Entity& other) const { return name < other.name; }

    /**
     * @brief Resolve names in the entity.
     */
    void resolve();

    virtual void expand_calls() {}

    void evaluate();
    [[nodiscard]] EvaluationResult evaluate(EvaluationContext::EvaluationType type);
    [[nodiscard]] bool check_unresolved(Unresolved& unresolved) const;

    [[nodiscard]] bool is_default_only() const { return default_only; }

    [[nodiscard]] bool is_public() const { return visibility == Visibility::PUBLIC; }

    void uses(Entity* entity);

    [[nodiscard]] Symbol get_name() const { return name; }

    [[nodiscard]] Location get_location() const { return location; }

    [[nodiscard]] Visibility get_visibility() const { return visibility; }

    [[nodiscard]] std::shared_ptr<Scope> get_scope() const { return scope; }

    void virtual serialize(std::ostream& stream) const { serialize_entity(stream); }

    // TODO: make protected once BodyParser doesn't evaluate body directly.
    void process_result(EvaluationResult& result);

    /// @brief The name of the entity.
    Symbol name;

    /// @brief The location of the entity in the source code.
    Location location;

    /// @brief The visibility of the entity, i. e. in which scope it can be used.
    Visibility visibility;

    /// @brief All entities referenced by the entity.
    std::unordered_set<Entity*> referenced_entities;

    /// @brief The entity's scope.
    std::shared_ptr<Scope> scope;

  protected:
    /*
     * @brief Resolve names in the entity.
     *
     * Subclasses must implement this method.
     */
    virtual void resolve_implementation() = 0;


    void serialize_entity(std::ostream& stream) const;

    [[nodiscard]] virtual EvaluationContext evaluation_context(EvaluationResult& result) { return EvaluationContext(result, this); }

    [[nodiscard]] virtual EvaluationContext evaluation_context(EvaluationResult& result, EvaluationContext::EvaluationType type) { return EvaluationContext(result, type, scope); }

    virtual void evaluate_inner(EvaluationContext& context) = 0;

  private:
    [[nodiscard]] bool check_unresolved(const std::unordered_set<Symbol>& unresolved, Unresolved::Part& part) const;

    static const Token token_default_only;
    static const Token token_visibility;

    bool default_only{false};

    // @brief Whether the entity has been resolved.
    bool resolved{false};
};

std::ostream& operator<<(std::ostream& stream, const Entity& entity);

#endif // HAD_XLR8_ENTITY_H
#undef IN_XLR8_ENTITY_H
