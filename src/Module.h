#ifdef IN_XLR8_MODULE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_MODULE_H
#ifndef HAD_XLR8_MODULE_H
#define HAD_XLR8_MODULE_H

/*
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

#include <memory>
#include <unordered_map>

#include "Scope.h"

class Module {
  public:
    Module() : Module(Symbol()) {}

    Module(Symbol name);

    /**
     * @brief Add a file to the module.
     *
     * @param file_name The name of the file.
     * @return The file scope of the new file.
     */
    [[nodiscard]] std::shared_ptr<Scope> add_file(Symbol file_name);

    /**
     * @brief Get the file scope for a specific file.
     *
     * @param file_name The name of the file.
     * @return The file scope.
     */
    [[nodiscard]] std::shared_ptr<Scope> file_scope(Symbol file_name) const;

    /**
     * @brief Get the module's public scope.
     *
     * @return The public scope.
     */
    [[nodiscard]] std::shared_ptr<Scope> public_scope() const { return public_scope_; }

    /**
     * @brief Get the module's private scope.
     *
     * @return The private scope.
     */
    [[nodiscard]] std::shared_ptr<Scope> private_scope() const { return private_scope_; }

    /**
     * @brief Import a module.
     *
     * @param visibility The visibility of the imported symbols.
     * @param module The module to import.
     */
    void import(Visibility visibility, const Module& module);

    void add_entity(std::shared_ptr<Entity> entity, std::shared_ptr<Scope> current_file_scope);

    /*
     * Get the name of the module.
     *
     * @return The name of the module.
     */
    Symbol name() const { return name_; }

    /**
     * @brief Get all constants defined in the module.
     *
     * @return A map of constant names to their values.
     */
    [[nodiscard]] std::vector<Constant*> constants() const { return get_entities<Constant>(); }

    /**
     * @brief Get all functions defined in the module.
     *
     * @return A vector of functions.
     */
    [[nodiscard]] std::vector<Function*> functions() const { return get_entities<Function>(); }

    /**
     * @brief Get all macros defined in the module.
     *
     * @return A vector of macros.
     */
    [[nodiscard]] std::vector<Macro*> macros() const { return get_entities<Macro>(); }

    /**
     * @brief Get all objects defined in the module.
     *
     * @return A vector of objects.
     */
    [[nodiscard]] std::vector<Object*> objects() const { return get_entities<Object>(); }

    /**
     * @brief Get all entities defined in the module.
     *
     * @return A vector of entities.
     */
    [[nodiscard]] std::vector<Entity*> entities() const;

    [[nodiscard]] std::vector<Entity*> explicitly_used_entities() const;

    template <typename T> std::vector<T*> get_entities() const {
        std::vector<T*> entities;
        for (const auto& entity : contained_entities) {
            if (auto casted_entity = entity->as<T>()) {
                entities.push_back(casted_entity);
            }
        }
        return entities;
    }

    /*
     * @brief Rename the module.
     *
     * @param new_name The new name of the module.
     */
    void rename(Symbol new_name);

    // Remove this? It's used to pass the target from a source file via Assembler to Linker.
    /// @brief The target of the module.
    const Target* target{};


  private:
    std::unordered_set<std::shared_ptr<Entity>> contained_entities;

    /// @brief The name of the module.
    Symbol name_;

    // @brief The file scopes of the module's files.
    std::unordered_map<Symbol, std::shared_ptr<Scope>> file_scopes;

    /// @brief The public scope of the module.
    std::shared_ptr<Scope> public_scope_;

    /// @brief The private scope of the module.
    std::shared_ptr<Scope> private_scope_;
};


#endif // HAD_XLR8_MODULE_H
#undef IN_XLR8_MODULE_H
