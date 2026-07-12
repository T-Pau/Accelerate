#ifndef HAD_XLR8_CALLABLE_H
#define HAD_XLR8_CALLABLE_H

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

#include "Entity/Entity.h"
#include "EvaluationContext.h"
#include "Expression/Expression.h"

/**
  * @brief Represents a callable entity that can be invoked with arguments: a macro or function.
 */
class Callable: public Entity {
  public:
    /**
     * @brief Represents the argument definitions of a callable entity.
     *
     * Each argument has a name. If the argument is optional, it also has a default value. No non-optional arguments can follow optional arguments.
     */
    class Arguments {
      public:
        /// @brief Construct an empty Arguments object.
        Arguments() = default;

        /// @brief Construct an Arguments object from a tokenizer.
        explicit Arguments(Tokenizer &tokenizer);

        /**
         * @brief Add an argument to the Arguments object.
         *
         * @param name The name of the argument.
         * @param default_argument The default value of the argument, if any.
         */
        void add(Symbol name, std::optional<Expression> default_argument);

        /**
         * @brief Check if callable takes no arguments.
         *
         * @return `true` if the callable takes no arguments, `false` otherwise.
         */
        [[nodiscard]] bool empty() const {return names.empty();}
        
        /**
         * @brief Get the name of the argument at a given index.
         *
         * @param index The index of the argument.
         * @return The name of the argument.
         */
        [[nodiscard]] Symbol name(size_t index) const {return names[index];}

        /**
         * @brief Get the list of argument names.
         *
         * @return The list of argument names.
         */
        [[nodiscard]] const std::vector<Symbol>& argument_names() const {return names;}

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
        [[nodiscard]] size_t maximum_arguments() const {return names.size();}

        /**
         * @brief Get the minimum number of arguments the callable requires.
         *
         * @return The minimum number of arguments.
         */
        [[nodiscard]] size_t minimum_arguments() const {return names.size() - default_arguments.size();}
        
        /**
         * @brief Print the Arguments object to a stream.
         *
         * @param stream The stream to print to.
         */
        void serialize(std::ostream& stream) const;

        /// @brief The names of the arguments.
        std::vector<Symbol> names;

        /**
         * @brief The default values of the arguments.
         *
         * The index in this array is offset by the number of non-optional arguments.
         */
        std::vector<Expression> default_arguments;
    };

    /**
     * @brief Construct a new Callable object from a library.
     *
     * @param owner The owner of the callable.
     * @param name The name of the callable.
     * @param definition The definition of the callable.
     */
    Callable(const Location& location, Symbol name, std::shared_ptr<Scope> parent_scope, const std::shared_ptr<StructuredValue>& definition);

    /**
     * @brief Construct a new Callable object.
     *
     * @param owner The owner of the callable.
     * @param name The name of the callable.
     * @param visibility The visibility of the callable.
     * @param default_only Whether this definition can be overridden by a subsequent definition.
     * @param arguments The argument definitions of the callable.
     */
    Callable(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> parent_scope, bool default_only, Arguments arguments): Entity(location, name, visibility, parent_scope, default_only), arguments(std::move(arguments)) {}

    /**
     * @brief Get the name of the argument at a given index.
     *
     * @param index The index of the argument.
     * @return The name of the argument.
     */
    [[nodiscard]] Symbol argument_name(size_t index) const {return arguments.name(index);}

    /**
     * @brief Get the default value of the argument at a given index.
     *
     * @param index The index of the argument.
     * @return The default value of the argument, if any.
     */
    [[nodiscard]] std::optional<Expression> default_argument(size_t index) const {return arguments.default_argument(index);}

    /**
     * @brief Get the list of argument names.
     *
     * @return The list of argument names.
     */
    [[nodiscard]] const std::vector<Symbol>& argument_names() const {return arguments.argument_names();}

  protected:
    /// @brief The argument definitions.
    Arguments arguments;

    [[nodiscard]] EvaluationContext evaluation_context(EvaluationResult& result) override;

    void serialize_callable(std::ostream& stream) const;

  private:
    /// @brief Initialize static members.
    static void initialize();

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
std::ostream& operator<<(std::ostream& stream, const Callable::Arguments& arguments);

#endif // HAD_XLR8_CALLABLE_H
