#ifdef IN_XLR8_CALLABLE_ARGUMENTS_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_CALLABLE_ARGUMENTS_H
#ifndef HAD_XLR8_CALLABLE_ARGUMENTS_H
#define HAD_XLR8_CALLABLE_ARGUMENTS_H

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

#include "Entity/ScopeEntity.h"
#include "Expression/ArgumentExpression.h"
#include "Expression/Expression.h"
#include "Tokenizer.h"

template <typename T>
concept ScopeBodyOrExpression = requires(T obj) {
    { obj.inner_scope() } -> std::same_as<std::shared_ptr<Scope>>;
    { obj.add(std::declval<std::shared_ptr<Constant>>()) } -> std::same_as<void>;
};

class CallableArguments {
  public:
    /// @brief Construct an empty Arguments object.
    CallableArguments() = default;

    explicit CallableArguments(ScopeEntity* entity, const std::shared_ptr<StructuredValue>& definition);

    /// @brief Construct an Arguments object from a tokenizer.
    explicit CallableArguments(Tokenizer& tokenizer);

    void set_entity(ScopeEntity* entity) { this->entity = entity; /* TODO: Check that it's not already set. */ }

    /**
     * @brief Check if callable takes no arguments.
     *
     * @return `true` if the callable takes no arguments, `false` otherwise.
     */
    [[nodiscard]] bool empty() const { return names.empty(); }

    /**
     * @brief Get the name of the argument at a given index.
     *
     * @param index The index of the argument.
     * @return The name of the argument.
     */
    [[nodiscard]] Symbol name(size_t index) const { return names[index]; }

    /**
     * @brief Get the list of argument names.
     *
     * @return The list of argument names.
     */
    [[nodiscard]] const std::vector<Symbol>& argument_names() const { return names; }

    /**
     * @brief Get the default value of the argument at a given index.
     *
     * @param index The index of the argument.
     * @return The default value of the argument, if any.
     */
    [[nodiscard]] std::optional<Expression> default_argument(size_t index) const;

    /**
     * @brief Get the maximum number of arguments the callable can take.
     *
     * @return The maximum number of arguments.
     */
    [[nodiscard]] size_t maximum_arguments() const { return names.size(); }

    /**
     * @brief Get the minimum number of arguments the callable requires.
     *
     * @return The minimum number of arguments.
     */
    [[nodiscard]] size_t minimum_arguments() const { return names.size() - default_arguments.size(); }

    void traverse(std::function<void(Expression&)> expression_callback);

    /**
     * @brief Print the Arguments object to a stream.
     *
     * @param stream The stream to print to.
     */
    void serialize(std::ostream& stream) const;

    void serialize_callable(std::ostream& stream) const;

    /// @brief The names of the arguments.
    std::vector<Symbol> names;

    /**
     * @brief The default values of the arguments.
     *
     * The index in this array is offset by the number of non-optional arguments.
     */
    std::vector<Expression> default_arguments;

    /// @brief Enter arguments into the callable's scope.
    void enter_arguments();

    /**
     * @brief Set the arguments for a callable entity.
     *
     * This creates constants for the actual arguments and sets them in the ArgumentExpression placeholders.
     *
     * Call this function before cloning the callable so the actual arguments will be included in the expanded copy.
     *
     * @tparam Expansion The type of the expanded copy (ScopeBody or ScopeExpression).
     * @param expansion The expanded copy.
     * @param arguments The list of argument expressions.
     */
    template <ScopeBodyOrExpression Expansion> void set_arguments(Expansion* expansion, const std::vector<Expression>& arguments) {
        auto constants = create_argument_constants(expansion->inner_scope(), arguments);
        for (auto& constant : constants) {
            expansion->add(constant);
        }
        set_argument_expressions(constants);
    }

    /**
     * @brief Clear the arguments for a callable entity.
     *
     * This clears the actual arguments in the ArgumentExpression placeholders.
     *
     * Call this function after cloning the callable to reset the arguments.
     */
    void clear_arguments();

  private:
    /// @brief Initialize static members.
    static void initialize();

    /**
     * @brief Get the argument expression for a given argument name.
     *
     * @param entity The ScopeEntity containing the argument.
     * @param name The name of the argument.
     * @return A pointer to the argument expression.
     */
    ArgumentExpression* get(ScopeEntity* entity, Symbol name) const;

    /**
     * @brief Create constants for the actual arguments and add default arguments as required.
     *
     * @param scope The scope to create the constants in.
     * @param arguments The list of argument expressions.
     * @return A vector of shared pointers to the created constants.
     */
    std::vector<std::shared_ptr<Constant>> create_argument_constants(std::shared_ptr<Scope> scope, const std::vector<Expression>& arguments) const;

    /**
     * @brief Set the ArgumentExpressions to their constants.
     *
     * @param constants The list of constants representing the arguments.
     */
    void set_argument_expressions(const std::vector<std::shared_ptr<Constant>>& constants);

    /// @brief The ScopeEntity the arguments belong to.
    ScopeEntity* entity{};

    /// @brief Whether static members have been initialized.
    static bool initialized;

    /// @brief The token representing the "arguments" key in a callable's representation in a library.
    static Token token_arguments;
};

/**
 * @brief Serialize the argument definitions to a stream.
 *
 * @param stream The output stream.
 * @param arguments The argument definitions.
 * @return The output stream.
 */
std::ostream& operator<<(std::ostream& stream, const CallableArguments& arguments);

#endif // HAD_XLR8_CALLABLE_ARGUMENTS_H
#undef IN_XLR8_CALLABLE_ARGUMENTS_H
