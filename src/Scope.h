#ifdef IN_XLR8_SCOPE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_SCOPE_H
#ifndef HAD_XLR8_SCOPE_H
#define HAD_XLR8_SCOPE_H

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

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/Exception.h>
#include <tpau-cpp-kernal/Symbol.h>
#include <tpau-cpp-kernal/Util.h>

#include "Entity/Constant.h"
#include "Entity/Function.h"
#include "Entity/Macro.h"
#include "Entity/Object.h"
#include "Expression/Expression.h"
#include "UnnamedLabelList.h"
#include "Visibility.h"

using namespace tpau::cpp_kernal;

class Module;

/**
 * @brief Represents a scope used for name resolution. It can be contained in other scopes.
 *
 * Lookups search all containing scopes going outwards.
 */
class Scope {
  public:
    class AllCollections {
      public:
        AllCollections(const Scope* scope) : scope(scope) {}

        class Iterator {
          public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::shared_ptr<Entity>;
            using difference_type = std::ptrdiff_t;
            using pointer = Entity**;
            using reference = Entity*&;

            Iterator(const Scope* scope, std::unordered_map<Symbol, std::weak_ptr<Constant>>::const_iterator constant_it, std::unordered_map<Symbol, std::weak_ptr<Function>>::const_iterator function_it, std::unordered_map<Symbol, std::weak_ptr<Macro>>::const_iterator macro_it, std::unordered_map<Symbol, std::weak_ptr<Object>>::const_iterator object_it) : scope(scope), constant_it(constant_it), function_it(function_it), macro_it(macro_it), object_it(object_it) {}

            [[nodiscard]] value_type operator*() const {
                if (constant_it != scope->constants.end()) {
                    auto entity = constant_it->second.lock();
                    if (!entity) {
                        throw Exception("internal error: entity {} in scope has expired", constant_it->first);
                    }
                    return std::dynamic_pointer_cast<Entity>(entity);
                }
                else if (function_it != scope->functions.end()) {
                    auto entity = function_it->second.lock();
                    if (!entity) {
                        throw Exception("internal error: entity {} in scope has expired", function_it->first);
                    }
                    return std::dynamic_pointer_cast<Entity>(entity);
                }
                else if (macro_it != scope->macros.end()) {
                    auto entity = macro_it->second.lock();
                    if (!entity) {
                        throw Exception("internal error: entity {} in scope has expired", macro_it->first);
                    }
                    return std::dynamic_pointer_cast<Entity>(entity);
                }
                else if (object_it != scope->objects.end()) {
                    auto entity = object_it->second.lock();
                    if (!entity) {
                        throw Exception("internal error: entity {} in scope has expired", object_it->first);
                    }
                    return std::dynamic_pointer_cast<Entity>(entity);
                }
                else {
                    throw Exception("iterator out of range");
                }
            }

            Iterator& operator++() {
                if (constant_it != scope->constants.end()) {
                    ++constant_it;
                }
                else if (function_it != scope->functions.end()) {
                    ++function_it;
                }
                else if (macro_it != scope->macros.end()) {
                    ++macro_it;
                }
                else if (object_it != scope->objects.end()) {
                    ++object_it;
                }
                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp(*this);
                ++(*this);
                return tmp;
            }

            bool operator==(const Iterator& other) const { return constant_it == other.constant_it && function_it == other.function_it && macro_it == other.macro_it && object_it == other.object_it && scope == other.scope; }

            bool operator!=(const Iterator& other) const { return !(*this == other); }

          private:
            const Scope* scope;
            std::unordered_map<Symbol, std::weak_ptr<Constant>>::const_iterator constant_it;
            std::unordered_map<Symbol, std::weak_ptr<Function>>::const_iterator function_it;
            std::unordered_map<Symbol, std::weak_ptr<Macro>>::const_iterator macro_it;
            std::unordered_map<Symbol, std::weak_ptr<Object>>::const_iterator object_it;
        };

        [[nodiscard]] Iterator begin() const { return Iterator(scope, scope->constants.begin(), scope->functions.begin(), scope->macros.begin(), scope->objects.begin()); }

        [[nodiscard]] Iterator end() const { return Iterator(scope, scope->constants.end(), scope->functions.end(), scope->macros.end(), scope->objects.end()); }

      private:
        const Scope* scope;
    };

    /**
     * @brief Represents an unordered collection of entities in the scope.
     *
     * It provides an iterator to iterate over the elements in the collection.
     */
    template <typename T> class Collection {
      public:
        /**
         * @brief Initialize the collection with the given unordered map.
         *
         * @param collection The unordered map containing the entities.
         */
        Collection(const std::unordered_map<Symbol, std::weak_ptr<T>>& collection) : collection(collection) {}

        /**
         * @brief Iterator for the collection.
         */
        class Iterator {
          public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::shared_ptr<T>;
            using difference_type = std::ptrdiff_t;
            using pointer = std::shared_ptr<T>*;
            using reference = std::shared_ptr<T>&;

            /**
             * @brief Initialize the iterator with the given unordered map iterator.
             *
             * @param it The unordered map iterator.
             */
            Iterator(std::unordered_map<Symbol, std::weak_ptr<T>>::const_iterator it) : it(it) {}

            /**
             * @brief Dereference the iterator.
             *
             * @return The entity pointed to by the iterator.
             */
            [[nodiscard]] value_type operator*() const {
                auto entity = it->second.lock();
                if (!entity) {
                    throw Exception("internal error: entity {} in scope has expired", it->first);
                }
                return entity;
            }

            /**
             * @brief Get the pointer to the entity pointed to by the iterator.
             *
             * @return The pointer to the entity.
             */
            [[nodiscard]] pointer operator->() const {
                auto entity = it->second.lock();
                if (!entity) {
                    throw Exception("internal error: entity {} in scope has expired", it->first);
                }
                return &entity;
            }

            /**
             * @brief Pre-increment the iterator.
             *
             * @return The incremented iterator.
             */
            Iterator& operator++() {
                ++it;
                return *this;
            }

            /**
             * @brief Post-increment the iterator.
             *
             * @return The iterator before incrementing.
             */
            Iterator operator++(int) {
                Iterator tmp(*this);
                ++it;
                return tmp;
            }

            /**
             * @brief Check if the iterator is equal to another iterator.
             *
             * @param other The other iterator to compare with.
             * @return `true` if the iterators are equal, `false` otherwise.
             */
            bool operator==(const Iterator& other) const { return it == other.it; }

            /**
             * @brief Check if the iterator is not equal to another iterator.
             *
             * @param other The other iterator to compare with.
             * @return `true` if the iterators are not equal, `false` otherwise.
             */
            bool operator!=(const Iterator& other) const { return it != other.it; }

          private:
            /// @brief The underlying unordered map iterator.
            std::unordered_map<Symbol, std::weak_ptr<T>>::const_iterator it;
        };

        /**
         * @brief Get the beginning iterator of the collection.
         *
         * @return The beginning iterator.
         */
        [[nodiscard]] Iterator begin() const { return Iterator(collection.begin()); }

        /**
         * @brief Get the end iterator of the collection.
         *
         * @return The end iterator.
         */
        [[nodiscard]] Iterator end() const { return Iterator(collection.end()); }

        /**
         * @brief Get the size of the collection.
         *
         * @return The number of entities in the collection.
         */
        [[nodiscard]] size_t size() const { return collection.size(); }

      private:
        /// @brief The underlying unordered map containing the entities.
        const std::unordered_map<Symbol, std::weak_ptr<T>>& collection;
    };

    /**
     * Constructs a scope not contained in another scope.
     *
     * @param type The type of the scope.
     * @param name The name of the scope.
     */
    explicit Scope(Visibility type, Symbol name = {});

    /**
     * @brief Construct an unnamed scope contained in another scope.
     *
     * @param type The type of the scope.
     * @param next The next scope.
     */
    Scope(Visibility type, std::shared_ptr<Scope> next) : Scope(type, Symbol(), next) {}

    /**
     * Constructs a named scope contained in another scope.
     *
     * @param type The type of the scope.
     * @param name The name of the scope.
     * @param next The next scope.
     */
    explicit Scope(Visibility type, Symbol name, std::shared_ptr<Scope> next);

    ~Scope();

    void add_entity(std::shared_ptr<Entity> entity);

    /**
     * Add a constant to the scope.
     *
     * @param visibility The visibility of the constant. This is used to determine which containing scopes to add the variable to.
     * @param name The name of the constant.
     * @param constant The constant to add.
     */
    void add(std::shared_ptr<Constant> constant);
    /**
     * Add a function to the scope.
     *
     * @param name The name of the function.
     * @param function The function to add.
     */
    void add(std::shared_ptr<Function> function);

    /**
     * Add a macro to the scope.
     *
     * @param name The name of the macro.
     * @param macro The macro to add.
     */
    void add(std::shared_ptr<Macro> macro);

    /**
     * Add an object to the scope.
     *
     * @param visibility The visibility of the object. This is used to determine which containing scopes to add the variable to.
     * @param name The name of the object.
     * @param object The object to add.
     */
    void add(std::shared_ptr<Object> object);

    /**
     * Add a new scope to the list of containing scopes.

     * @param new_next The new scope to add.
     */
    void add_next(std::shared_ptr<Scope> new_next);

    void add_unnamed_label(const Location& location, Expression label_expression) { unnamed_labels.add_label(location, label_expression); }

    /**
     * Import a module.
     *
     * @param visibility The visibility of the imported symbols.
     * @param module The module to import.
     */
    void import(Visibility visibility, const Module& module);

    /**
     * Get a constant from the scope.
     *
     * @param name The name of the constant.
     * @param include_containing_scopes If `true`, search in containing scopes as well; if `false`, only search in this scope.
     * @return The constant if it exists, nullptr otherwise.
     */
    [[nodiscard]] std::shared_ptr<Constant> get_constant(Symbol name, bool include_containing_scopes = true) const { return get<Constant>(name, include_containing_scopes); }

    /**
     * Get a function from the scope.
     *
     * @param name The name of the function.
     * @param include_containing_scopes If `true`, search in containing scopes as well; if `false`, only search in this scope.
     * @return The function if it exists, nullptr otherwise.
     */
    [[nodiscard]] std::shared_ptr<Function> get_function(Symbol name, bool include_containing_scopes = true) const { return get<Function>(name, include_containing_scopes); }

    /**
     * Get a macro from the scope.
     *
     * @param name The name of the macro.
     * @param include_containing_scopes If `true`, search in containing scopes as well; if `false`, only search in this scope.
     * @return The macro if it exists, nullptr otherwise.
     */
    [[nodiscard]] std::shared_ptr<Macro> get_macro(Symbol name, bool include_containing_scopes = true) const { return get<Macro>(name, include_containing_scopes); }

    /**
     * Get an object from the scope.
     *
     * @param name The name of the object.
     * @param include_containing_scopes If `true`, search in containing scopes as well; if `false`, only search in this scope.
     * @return The object if it exists, nullptr otherwise.
     */
    [[nodiscard]] std::shared_ptr<Object> get_object(Symbol name, bool include_containing_scopes = true) const { return get<Object>(name, include_containing_scopes); }

    [[nodiscard]] AllCollections get_entities() const { return AllCollections(this); }

    /**
     * Get the objects defined in this scope. It does not include objects defined in containing scopes.
     *
     * @return A collection of all objects.
     */
    [[nodiscard]] Collection<Object> get_objects() const { return Collection<Object>(objects); }

    /**
     * Get the constants defined in this scope. It does not include constants defined in containing scopes.
     *
     * @return A collection of all constants.
     */
    [[nodiscard]] Collection<Constant> get_constants() const { return Collection<Constant>(constants); }

    /**
     * Get the functions defined in this scope. It does not include functions defined in containing scopes.
     *
     * @return A collection of all functions.
     */
    [[nodiscard]] Collection<Function> get_functions() const { return Collection<Function>(functions); }

    /**
     * Get the macros defined in this scope. It does not include macros defined in containing scopes.
     *
     * @return A collection of all macros.
     */
    [[nodiscard]] Collection<Macro> get_macros() const { return Collection<Macro>(macros); }

    template <typename T> [[nodiscard]] Collection<T> get_all() const { throw Exception("internal error: get_all called for unknown type"); }

    template <> [[nodiscard]] Collection<Constant> get_all<Constant>() const { return get_constants(); }

    template <> [[nodiscard]] Collection<Function> get_all<Function>() const { return get_functions(); }

    template <> [[nodiscard]] Collection<Macro> get_all<Macro>() const { return get_macros(); }

    template <> [[nodiscard]] Collection<Object> get_all<Object>() const { return get_objects(); }

    /**
     * Get the number of unnamed labels in the scope.
     *
     * @return The number of unnamed labels.
     */
    [[nodiscard]] size_t unnamed_label_count() const { return unnamed_labels.size(); }

    /**
     * Get the next unnamed label after a location.
     *
     * @param location The location to get the next unnamed label after.
     * @return The next unnamed label after the location if it exists, {} otherwise.
     */
    [[nodiscard]] std::optional<Expression> get_next_unnamed_label(const Location& location) const { return unnamed_labels.get_next_label(location); }

    /**
     * Get the previous unnamed label before a location.
     *
     * @param location The location to get the previous unnamed label before.
     * @return The previous unnamed label before the location if it exists, {} otherwise.
     */
    [[nodiscard]] std::optional<Expression> get_previous_unnamed_label(const Location& location) const { return unnamed_labels.get_previous_label(location); }

    /**
     * Check if a preprocessor symbol is defined in the scope.
     *
     * @param name The name of the symbol.
     * @return `true` if the symbol is defined, `false` otherwise.
     */
    [[nodiscard]] bool is_defined(Symbol name) const;

    /**
     * Undefine a preprocessor symbol in the scope.
     *
     * This will not undef the symbol in parent scopes.
     *
     * @param name The name of the symbol.
     */
    void undefine(Symbol name) {
        defines.erase(name);
        undefine_overrides.insert(name);
    }

    /**
     * Define a preprocessor symbol in the scope.
     *
     * @param name The name of the symbol.
     */
    void define(Symbol name) {
        defines.insert(name);
        undefine_overrides.erase(name);
    }

    /**
     * Define multiple preprocessor symbols in the scope.
     *
     * @param names The names of the symbols.
     */
    void define(const std::unordered_set<Symbol>& names) {
        defines.insert(names.begin(), names.end());
        for (const auto& name : names) {
            undefine_overrides.erase(name);
        }
    }

    /**
     * Pin an object to a specific address.
     *
     * @param object_name The name of the object.
     * @param address The address to pin the object to.
     */
    void pin(Symbol object_name, Expression address);

    /**
     * Explicitly mark an object as used.
     *
     * This will ensure that the object is included in the final output, even if it is not referenced by any other used entities.
     *
     * @param object_name The name of the object to mark as used.
     */
    void mark_used(Symbol object_name) { explicitly_used_object_names.insert(object_name); }

    /**
     * Explicitly mark an object as used.
     *
     * This will ensure that the object is included in the final output, even if it is not referenced by any other used entities.
     *
     * @param object The object to mark as used.
     */
    void mark_used(Object* object) { explicitly_used_objects.insert(object); }

    void remove_constant(Symbol name) { constants.erase(name); }

    /**
     * Get the type of the scope.
     *
     * @return The type of the scope.
     */
    [[nodiscard]] Visibility type() const { return type_; }

    [[nodiscard]] std::shared_ptr<Scope> parent() const { return next.empty() ? nullptr : next[0]; }

    /**
     * Get the name of the scope.
     *
     * @return The name of the scope.
     */
    [[nodiscard]] Symbol name() const { return name_; }

    [[nodiscard]] bool can_rename(Symbol new_name) const;

    /**
     * Rename the scope.
     *
     * @param new_name The new name of the scope.
     */
    void rename(Symbol new_name);

    /**
     * @brief Get the set of objects explicitly marked as used.
     *
     * @return The set of explicitly used objects.
     */
    [[nodiscard]] const std::unordered_set<Object*>& get_explicitly_used_objects() const { return explicitly_used_objects; }


  private:
    /**
     * @brief Determines if a scope of type `contained` can be contained in a scope of type `container`.
     *
     * @param container The type of the containing scope.
     * @param contained The type of the contained scope.
     * @return `true` if a scope of type `contained` can be contained in a scope of type `container`, `false` otherwise.
     */
    [[nodiscard]] static bool can_contain(Visibility container, Visibility contained);

    /**
     * @brief Checks that adding an entity of the given type and name is allowed and find the correct scope to add it to.
     *
     * If any errors are found, they will be reported via DiagnosticOutput and an exception will be thrown.
     *
     * @tparam T The type of the entity.
     * @param visibility The visibility of the entity.
     * @param name The name of the entity.
     * @param location The location of the entity definition in the source code.
     * @return The scope to add the entity to.
     * @throws Exception if adding the entity is not allowed.
     */
    template <typename T> Scope* add_precheck(Visibility visibility, Symbol name, const Location& location) {
        if ((visibility == Visibility::ENTITY || visibility == Visibility::ARGUMENT) && !std::is_same_v<T, Constant>) {
            DiagnosticOutput::global.error(location, "cannot add {} to scope of type {}", get_type_name<T>(), visibility);
            throw Exception();
        }
        if (auto conflicting = find_conflicting<T>(name, visibility)) {
            DiagnosticOutput::global.error(location, "{} '{}' already defined", get_type_name<T>(), name);
            DiagnosticOutput::global.note(*conflicting, "previous definition here");
            throw Exception();
        }
        auto scope = find_containing_scope(visibility);
        if (!scope) {
            DiagnosticOutput::global.error(location, "cannot find containing scope of type {}", visibility);
            throw Exception();
        }
        return scope;
    }

    /**
     * @brief Finds a conflicting entity in the scope.
     *
     * This is a helper function used in the `add_precheck()` template function.
     *
     * Since we cannot fully declare the entity classes here (they contain a Scope), we use specific functions for each entity type that are defined in the .cc file.
     *
     * @tparam T The type of the entity.
     * @param name The name of the entity.
     * @return The location of the conflicting entity, or {} if no conflict exists.
     */
    template <typename T> const std::optional<Location> find_conflicting(Symbol name, Visibility visibility) { throw Exception("internal error: find_conflicting() not defined for type {}", typeid(T).name()); }

    template <> const std::optional<Location> find_conflicting<Constant>(Symbol name, Visibility visibility) { return find_conflicting_constant_or_object(name, visibility); }

    template <> const std::optional<Location> find_conflicting<Function>(Symbol name, Visibility visibility) { return find_conflicting_function(name); }

    template <> const std::optional<Location> find_conflicting<Macro>(Symbol name, Visibility visibility) { return find_conflicting_macro(name); }

    template <> const std::optional<Location> find_conflicting<Object>(Symbol name, Visibility visibility) { return find_conflicting_constant_or_object(name, visibility); }

    std::optional<Location> find_conflicting_constant_or_object(Symbol name, Visibility visibility);
    std::optional<Location> find_conflicting_function(Symbol name);
    std::optional<Location> find_conflicting_macro(Symbol name);

    /**
     * @brief Gets the type name of an entity for error reporting.
     *
     * This is a helper function used in the `add()` template function.
     *
     * @tparam T The type of the entity.
     * @return The name of the entity type.
     */
    template <typename T> const char* get_type_name() { throw Exception("internal error: get_type_name() not defined for type {}", typeid(T).name()); }

    template <> const char* get_type_name<Constant>() { return "constant or object"; }

    template <> const char* get_type_name<Function>() { return "function"; }

    template <> const char* get_type_name<Macro>() { return "macro"; }

    template <> const char* get_type_name<Object>() { return "constant or object"; }

    /**
     * @brief Find an entity of the given type in the scope.
     *
     * This implements the logic for finding an entity. It is called by the public get functions for each entity type.
     *
     * @tparam T The type of the entity.
     * @param name The name of the entity.
     * @param include_containing_scopes If `true`, search in containing scopes as well; if `false`, only search in this scope.
     * @return The entity, if found; otherwise, nullptr.
     */
    template <typename T> std::shared_ptr<T> get(Symbol name, bool include_containing_scopes = true) const {
        if (auto entity = get_directly<T>(name)) {
            return entity;
        }
        if (include_containing_scopes) {
            for (auto& scope : next) {
                if (auto entity = scope->get<T>(name, include_containing_scopes)) {
                    return entity;
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief Find an entity of a given type defined in this scope, not looking in containing scopes.
     *
     * This is a helper function used in the `get()` template function.
     *
     * @tparam T The type of the entity.
     * @param name The name of the entity.
     * @return The entity, if found; otherwise, nullptr.
     */
    template <typename T> std::shared_ptr<T> get_directly(Symbol name) const { throw Exception("internal error: get() not defined for type {}", typeid(T).name()); }

    template <> std::shared_ptr<Constant> get_directly<Constant>(Symbol name) const { return get(constants, name); }

    template <> std::shared_ptr<Function> get_directly<Function>(Symbol name) const { return get(functions, name); }

    template <> std::shared_ptr<Macro> get_directly<Macro>(Symbol name) const { return get(macros, name); }

    template <> std::shared_ptr<Object> get_directly<Object>(Symbol name) const { return get(objects, name); }

    /**
     * @brief Get an entity from a collection by name.
     *
     * @tparam T The type of the entity.
     * @param collection The collection to search.
     * @param name The name of the entity.
     * @return The entity if found, {} otherwise.
     */
    template <typename T> std::shared_ptr<T> get(const std::unordered_map<Symbol, std::weak_ptr<T>>& collection, Symbol name) const {
        auto it = collection.find(name);
        if (it != collection.end()) {
            auto entity = it->second.lock();
            if (!entity) {
                throw Exception("internal error: entity {} in scope has expired", name);
            }
            return entity;
        }
        else {
            return nullptr;
        }
    }

    /**
     * @brief Finds the first containing scope of a given visibility.
     *
     * @param visibility The visibility of the containing scope to find.
     * @return The containing scope.
     * @throws Exception if no containing scope of the given visibility is found.
     */
    [[nodiscard]] Scope* find_containing_scope(Visibility visibility);

    /// @brief The name of the scope.
    Symbol name_;

    /// @brief The type of the scope.
    Visibility type_{Visibility::ENTITY};

    /// @brief The constants in the environment.
    std::unordered_map<Symbol, std::weak_ptr<Constant>> constants;

    /// @brief The functions in the environment.
    std::unordered_map<Symbol, std::weak_ptr<Function>> functions;

    /// @brief The macros in the environment.
    std::unordered_map<Symbol, std::weak_ptr<Macro>> macros;

    /// @brief The objects in the environment.
    std::unordered_map<Symbol, std::weak_ptr<Object>> objects;

    /// @brief The scopes to search for symbols not found in this scope.
    std::vector<std::shared_ptr<Scope>> next;

    /// @brief The named scopes contained in this scope.
    std::unordered_map<Symbol, Scope*> contained;

    /// @brief The preprocessor defines in the scope.
    std::unordered_set<Symbol> defines;

    /**
     * @brief The preprocessor defines explicitly undefined in this scope.
     *
     * This is used to override defines in parent scopes.
     */
    std::unordered_set<Symbol> undefine_overrides;

    /// @brief List of unnamed labels in the scope. This is only allowed in entity scopes.
    UnnamedLabelList unnamed_labels;

    /**
     * @brief The objects that have been pinned to specific addresses.
     *
     * These will be resolved and the address of the objects set once all names are defined.
     */
    std::unordered_map<Symbol, Expression> pinned_object_names;

    /**
     * @brief The names of the objects that have been explicitly marked as used.
     *
     * These will be resolved and added to `explicitly_used_objects` once all names are defined.
     */
    std::unordered_set<Symbol> explicitly_used_object_names;

    /// @brief The objects that have been explicitly marked as used.
    std::unordered_set<Object*> explicitly_used_objects;
};


#endif // HAD_XLR8_SCOPE_H
#undef IN_XLR8_SCOPE_H
