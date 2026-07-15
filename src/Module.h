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

    /**
     * @brief Pin an object to a specific address.
     *
     * @param name The name of the object.
     * @param address The address to pin the object to.
     */
    void pin(Symbol object_name, Expression address);

    /**
     * @brief Explicitly mark an object as used.
     *
     * This will ensure that the object is included in the final output, even if it is not referenced by any other used entities.
     *
     * @param name The name of the object.
     */
    void mark_used(Symbol name) { explicitly_used_object_names.insert(name); }

    /**
     * @brief Explicitly mark an object as used.
     *
     * This will ensure that the object is included in the final output, even if it is not referenced by any other used entities.
     *
     * @param object The object to mark as used.
     */
    void mark_used(Object* object) { explicitly_used_objects.insert(object); }

    /// @brief The name of the module.
    Symbol name;

    // Remove this? It's used to pass the target from a source file via Assembler to Linker.
    /// @brief The target of the module.
    const Target* target{};

  private:
    class Pinned {
      public:
        Pinned(Symbol object_name, Expression address) : object_name{object_name}, address{std::move(address)} {}

        Pinned() = default;

        Symbol object_name;
        Expression address;

        void resolve(Scope& scope);
        void evaluate(EvaluationContext context);
    };

    // @brief The file scopes of the module's files.
    std::unordered_map<Symbol, std::shared_ptr<Scope>> file_scopes;

    /// @brief The public scope of the module.
    std::shared_ptr<Scope> public_scope_;

    /// @brief The private scope of the module.
    std::shared_ptr<Scope> private_scope_;

    /// @brief The objects that have been pinned to specific addresses.
    std::unordered_map<Symbol, Pinned> pinned_objects;

    /**
     * @brief The names of the objects that have been explicitly marked as used.
     *
     * These will be resolved and added to `explicitly_used_objects` once all names are defined.
     */
    std::unordered_set<Symbol> explicitly_used_object_names;

    /// @brief The objects that have been explicitly marked as used.
    std::unordered_set<Object*> explicitly_used_objects;
};


#endif // HAD_XLR8_MODULE_H
#undef IN_XLR8_MODULE_H
