#include "Entity/ScopeEntity.h"

#include "Module.h"
#include "Scope.h"

ScopeEntity::ScopeEntity(const Location& location, Symbol name, std::shared_ptr<Scope> containing_scope, const std::shared_ptr<StructuredValue>& definition) : Entity(location, name, std::move(containing_scope), definition) { scope_ = std::make_shared<Scope>(Visibility::ENTITY, this->containing_scope()); }

ScopeEntity::ScopeEntity(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> containing_scope, bool default_only) : Entity(location, name, visibility, std::move(containing_scope), default_only) { scope_ = std::make_shared<Scope>(Visibility::ENTITY, this->containing_scope()); }

void ScopeEntity::add(std::shared_ptr<Constant> constant, bool add_to_scope) {
    if (constant->visibility != Visibility::ENTITY) {
        if (!add_to_scope) {
            throw LocationException(location, "internal error: cannot add constant {} with visibility {} to module but not to scope", constant->name, constant->visibility, type_name());
        }
        if (containing_module) {
            containing_module->add_entity(constant, scope()->parent());
        }
        else {
            throw LocationException(location, "internal error: cannot add constant {} with visibility {} to {} with no containing module", constant->name, constant->visibility, type_name());
        }
    }
    else {
        if (add_to_scope) {
            scope()->add(constant);
        }
        constants.insert(constant);
    }
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
