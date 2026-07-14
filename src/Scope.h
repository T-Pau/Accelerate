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

#include "Entity/Constant.h"
#include "Entity/Function.h"
#include "Entity/Macro.h"
#include "Entity/Object.h"
#include "Expression/Expression.h"
#include "UnnamedLabelList.h"
#include "Visibility.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a scope used for name resolution. It can be contained in other scopes.
 *
 * Lookups search all containing scopes going outwards.
 */
class Scope {
public:
    /**
     * @brief Represents an unordered collection of entities in the scope.
     *
     * It provides an iterator to iterate over the elements in the collection.
     */
    template<typename T>
    class Collection {
      public:
        /**
         * @brief Initialize the collection with the given unordered map.
         *
         * @param collection The unordered map containing the entities.
         */
        Collection(const std::unordered_map<Symbol, std::unique_ptr<T>>& collection): collection(collection) {}

        /**
         * @brief Iterator for the collection.
         */
        class Iterator {
          public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T*;
            using difference_type = std::ptrdiff_t;
            using pointer = T**;
            using reference = T*&;

            /**
             * @brief Initialize the iterator with the given unordered map iterator.
             *
             * @param it The unordered map iterator.
             */
            Iterator(std::unordered_map<Symbol, std::unique_ptr<T>>::const_iterator it): it(it) {}

            /**
             * @brief Dereference the iterator.
             *
             * @return The entity pointed to by the iterator.
             */
            [[nodiscard]] value_type operator*() const {return it->second.get();}

            /**
             * @brief Get the pointer to the entity pointed to by the iterator.
             *
             * @return The pointer to the entity.
             */
            [[nodiscard]] pointer operator->() const {return &(it->second.get());}

            /**
             * @brief Pre-increment the iterator.
             *
             * @return The incremented iterator.
             */
            Iterator& operator++() {++it; return *this;}

            /**
             * @brief Post-increment the iterator.
             *
             * @return The iterator before incrementing.
             */
            Iterator operator++(int) {Iterator tmp(*this); ++it; return tmp;}

            /**
             * @brief Check if the iterator is equal to another iterator.
             *
             * @param other The other iterator to compare with.
             * @return `true` if the iterators are equal, `false` otherwise.
             */
            bool operator==(const Iterator& other) const {return it == other.it;}

            /**
             * @brief Check if the iterator is not equal to another iterator.
             *
             * @param other The other iterator to compare with.
             * @return `true` if the iterators are not equal, `false` otherwise.
             */
            bool operator!=(const Iterator& other) const {return it != other.it;}

          private:
            /// @brief The underlying unordered map iterator.
            std::unordered_map<Symbol, std::unique_ptr<T>>::const_iterator it;
        };

        /**
         * @brief Get the beginning iterator of the collection.
         *
         * @return The beginning iterator.
         */
        [[nodiscard]] Iterator begin() const {return Iterator(collection.begin());}

        /**
         * @brief Get the end iterator of the collection.
         *
         * @return The end iterator.
         */
        [[nodiscard]] Iterator end() const {return Iterator(collection.end());}

        /**
         * @brief Get the size of the collection.
         *
         * @return The number of entities in the collection.
         */
        [[nodiscard]] size_t size() const {return collection.size();}

      private:
        /// @brief The underlying unordered map containing the entities.
        const std::unordered_map<Symbol, std::unique_ptr<T>>& collection;
    };

    /**
     * Constructs a scope not contained in another scope.
     */
    explicit Scope(Visibility type);

    /**
     * Constructs a scope contained in another scope.
     *
     * @param next The next scope.
     */
    explicit Scope(Visibility type, std::shared_ptr<Scope> next);

    /**
     * Add a constant to the scope.
     *
     * @param visibility The visibility of the constant. This is used to determine which containing scopes to add the variable to.
     * @param name The name of the constant.
     * @param constant The constant to add.
     */
    void add(std::unique_ptr<Constant> constant);
    /**
     * Add a function to the scope.
     *
     * @param name The name of the function.
     * @param function The function to add.
     */
    void add(std::unique_ptr<Function> function);

    /**
     * Add a macro to the scope.
     *
     * @param name The name of the macro.
     * @param macro The macro to add.
     */
    void add(std::unique_ptr<Macro> macro);

    /**
     * Add an object to the scope.
     *
     * @param visibility The visibility of the object. This is used to determine which containing scopes to add the variable to.
     * @param name The name of the object.
     * @param object The object to add.
     */
    void add(std::unique_ptr<Object> object);

    /**
     * Add a new scope to the list of containing scopes.

     * @param new_next The new scope to add.
     */
    void add_next(std::shared_ptr<Scope> new_next);

    /**
     * Get a constant from the scope.
     * 
     * @param name The name of the constant.
     * @return The constant if it exists, nullptr otherwise.
     */
    [[nodiscard]] Constant* get_constant(Symbol name) const {return get<Constant>(name);}

    /**
     * Get a function from the scope.
     *
     * @param name The name of the function.
     * @return The function if it exists, nullptr otherwise.
     */
    [[nodiscard]] Function* get_function(Symbol name) const {return get<Function>(name);}

    /**
     * Get a macro from the scope.
     *
     * @param name The name of the macro.
     * @return The macro if it exists, nullptr otherwise.
     */
    [[nodiscard]] Macro* get_macro(Symbol name) const {return get<Macro>(name);}

    /**
     * Get an object from the scope.
     *
     * @param name The name of the object.
     * @return The object if it exists, nullptr otherwise.
     */
    [[nodiscard]] Object* get_object(Symbol name) const {return get<Object>(name);}

    /**
     * Get the objects defined in this scope. It does not include objects defined in containing scopes.
     *
     * @return A collection of all objects.
     */
    [[nodiscard]] Collection<Object> get_objects() const {return Collection<Object>(objects);}
    
    /**
     * Get the constants defined in this scope. It does not include constants defined in containing scopes.
     *
     * @return A collection of all constants.
     */
    [[nodiscard]] Collection<Constant> get_constants() const {return Collection<Constant>(constants);}
    
    /**
     * Get the functions defined in this scope. It does not include functions defined in containing scopes.
     *
     * @return A collection of all functions.
     */
    [[nodiscard]] Collection<Function> get_functions() const {return Collection<Function>(functions);}
    
    /**
     * Get the macros defined in this scope. It does not include macros defined in containing scopes.
     *
     * @return A collection of all macros.
     */
    [[nodiscard]] Collection<Macro> get_macros() const {return Collection<Macro>(macros);}

    /**
     * Get the number of unnamed labels in the scope.
     *
     * @return The number of unnamed labels.
     */
    [[nodiscard]] size_t unnamed_label_count() const {return unnamed_labels.size();}

    /**
     * Get the next unnamed label after a location.
     *
     * @param location The location to get the next unnamed label after.
     * @return The next unnamed label after the location if it exists, {} otherwise.
     */
    [[nodiscard]] std::optional<Expression> get_next_unnamed_label(const Location& location) const {return unnamed_labels.get_next_label (location);}

    /**
     * Get the previous unnamed label before a location.
     *
     * @param location The location to get the previous unnamed label before.
     * @return The previous unnamed label before the location if it exists, {} otherwise.
     */
    [[nodiscard]] std::optional<Expression> get_previous_unnamed_label(const Location& location) const {return unnamed_labels.get_previous_label(location);}

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
    void undefine(Symbol name) {defines.erase(name); undefine_overrides.insert(name);}

    /**
     * Define a preprocessor symbol in the scope.
     *
     * @param name The name of the symbol.
     */
    void define(Symbol name) {defines.insert(name); undefine_overrides.erase(name);}

    /**
     * Get the type of the scope.
     *
     * @return The type of the scope.
     */
    [[nodiscard]] Visibility type() const {return type_;}

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
    template<typename T>
    Scope* add_precheck(Visibility visibility, Symbol name, const Location& location) {
        if ((visibility == Visibility::ENTITY || visibility == Visibility::SCOPE) && !std::is_same_v<T, Constant>) {
            DiagnosticOutput::global.error(location, "cannot add {} to scope of type {}", get_type_name<T>(), visibility);
            throw Exception();
        }
        if (auto conflicting = find_conflicting<T>(name)) {
            DiagnosticOutput::global.error(location, "{} {} already defined", get_type_name<T>(), name);
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
    template<typename T>
    const std::optional<Location> find_conflicting(Symbol name) {
        throw Exception("internal error: find_conflicting() not defined for type {}", typeid(T).name());
    }
    template<>
    const std::optional<Location> find_conflicting<Constant>(Symbol name) {return find_conflicting_constant_or_object(name);}
    template<>
    const std::optional<Location> find_conflicting<Function>(Symbol name) {return find_conflicting_function(name);}
    template<>
    const std::optional<Location> find_conflicting<Macro>(Symbol name) {return find_conflicting_macro(name);}
    template<>
    const std::optional<Location> find_conflicting<Object>(Symbol name) {return find_conflicting_constant_or_object(name);}
    std::optional<Location> find_conflicting_constant_or_object(Symbol name);
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
    template<typename T>
    const char* get_type_name() {
        throw Exception("internal error: get_type_name() not defined for type {}", typeid(T).name());
    }
    template<>
    const char* get_type_name<Constant>() {return "constant or object";}
    template<>
    const char* get_type_name<Function>() {return "function";}
    template<>
    const char* get_type_name<Macro>() {return "macro";}
    template<>
    const char* get_type_name<Object>() {return "constant or object";}

    /**
     * @brief Find an entity of the given type in the scope.
     *
     * This implements the logic for finding an entity. It is called by the public get functions for each entity type.
     *
     * @tparam T The type of the entity.
     * @param name The name of the entity.
     * @return The entity, if found; otherwise, nullptr.
     */
    template<typename T>
    T* get(Symbol name) const {
        if (auto entity = get_directly<T>(name)) {
            return entity;
        }
        for (auto& scope: next) {
            if (auto entity = scope->get<T>(name)) {
                return entity;
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
    template<typename T>
    T* get_directly(Symbol name) const {
        throw Exception("internal error: get() not defined for type {}", typeid(T).name());
    }
    template<>
    Constant* get_directly<Constant>(Symbol name) const {return constants.at(name).get();}
    template<>
    Function* get_directly<Function>(Symbol name) const {return functions.at(name).get();}
    template<>
    Macro* get_directly<Macro>(Symbol name) const {return macros.at(name).get();}
    template<>
    Object* get_directly<Object>(Symbol name) const {return objects.at(name).get();}

    /**
     * @brief Finds the first containing scope of a given visibility.
     *
     * @param visibility The visibility of the containing scope to find.
     * @return The containing scope.
     * @throws Exception if no containing scope of the given visibility is found.
     */
    [[nodiscard]] Scope* find_containing_scope(Visibility visibility);

    /// @brief The type of the scope.
    Visibility type_{Visibility::ENTITY};

    /// @brief The constants in the environment.
    std::unordered_map<Symbol, std::unique_ptr<Constant>> constants;

    /// @brief The functions in the environment.
    std::unordered_map<Symbol, std::unique_ptr<Function>> functions;

    /// @brief The macros in the environment.
    std::unordered_map<Symbol, std::unique_ptr<Macro>> macros;

    /// @brief The objects in the environment.
    std::unordered_map<Symbol, std::unique_ptr<Object>> objects;

    /// @brief The scopes to search for symbols not found in this scope.
    std::vector<std::shared_ptr<Scope>> next;

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
};


#endif // HAD_XLR8_SCOPE_H
#undef IN_XLR8_SCOPE_H
