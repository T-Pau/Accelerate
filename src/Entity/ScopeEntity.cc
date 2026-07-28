#include "Entity/ScopeEntity.h"

#include "Scope.h"

ScopeEntity::ScopeEntity(const Location& location, Symbol name, std::shared_ptr<Scope> containing_scope, const std::shared_ptr<StructuredValue>& definition) : Entity(location, name, std::move(containing_scope), definition) { scope_ = std::make_shared<Scope>(Visibility::ENTITY, this->containing_scope()); }

ScopeEntity::ScopeEntity(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> containing_scope, bool default_only) : Entity(location, name, visibility, std::move(containing_scope), default_only) { scope_ = std::make_shared<Scope>(Visibility::ENTITY, this->containing_scope()); }

void ScopeEntity::add(std::shared_ptr<Constant> constant) {
    if (constant->visibility != Visibility::ENTITY) {
        throw Exception("internal error: cannot add constant {} with visibility {} to {}, add to module instead", constant->name, constant->visibility, type_name());
    }
    scope()->add(constant);
    constants.insert(constant);
}

void ScopeEntity::traverse(std::function<void(Entity&)> entity_callable, std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    for (auto& constant : constants) {
        entity_callable(*constant);
    }
}

void ScopeEntity::resolve_constants() {
    for (auto& constant : constants) {
        constant->resolve();
    }
}
