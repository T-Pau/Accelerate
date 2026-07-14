#ifdef IN_XLR8_ADDRESSING_MODE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_ADDRESSING_MODE_H
#ifndef HAD_XLR8_ADDRESSING_MODE_H
#define HAD_XLR8_ADDRESSING_MODE_H

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

#include <utility>

#include <tpau-cpp-kernal/Symbol.h>

#include "ArgumentType.h"
#include "Body/Body.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents an addressing mode, which defines how an instruction's operands are specified and encoded.
 */
class AddressingMode {
public:
    /**
     * @brief Represents a notation for an instruction, which defines the syntax of the instruction.
     *
     * It includes a sequence of punctuation, reserved words, and arguments.
     */
    class Notation {
    public:
        /// @brief The type of an element in the notation.
        enum Type {
            /// @brief The element is an argument, corresponding to an operand of the instruction.
            ARGUMENT,

            /// @brief The element is punctuation.
            PUNCTUATION,

            /// @brief The element is a reserved word.
            RESERVED_WORD
        };

        /// @brief Represents an element in the notation, which can be punctuation, reserved word, or argument.
        class Element {
        public:
            /**
             * @brief Initialize an element in the notation with a specific type and symbol.
             *
             * @param type The type of the element.
             * @param symbol The symbol corresponding to the element, or the name of the argument.
             */
            Element(Type type, Symbol symbol): type(type), symbol(symbol) {}

            /**
             * @brief Check if the element is an argument.
             *
             * @return `true` if the element is an argument, `false` otherwise.
             */
            [[nodiscard]] bool is_argument() const {return type == ARGUMENT;}

            /// @brief The type of the element.
            Type type;

            /// @brief The symbol corresponding to the element, or the name of the argument.
            Symbol symbol;
        };

        /**
         * @brief Compute the names of the arguments in the notation.
         *
         * This method populates the `argument_names` vector based on the elements in the notation.
         */
        void compute_argument_names();

        /// @brief The elements that make up the notation.
        std::vector<Element> elements;

        /// @brief The names of the arguments in the notation, in order.
        std::vector<Symbol> argument_names;
    };

    /**
     * @brief Represents an argument for the instruction, which can have a specific type and an optional default value.
     */
    class Argument {
      public:
        /**
         * @brief Initialize an argument with a specific type and an optional default value.
         *
         * @param type The type of the argument.
         * @param default_value The optional default value of the argument.
         */
        explicit Argument(const ArgumentType* type, std::optional<Value> default_value = {}) : type{type}, default_value{std::move(default_value)} {}

        /// @brief Default constructor for Argument.
        Argument() = default;

        /// @brief The type of the argument.
        const ArgumentType* type{};

        /// @brief The optional default value of the argument.
        std::optional<Value> default_value;
    };

    /**
     * @brief Get the argument with the specified name.
     *
     * @param name The name of the argument.
     * @return A pointer to the argument if it exists, nullptr otherwise.
     */
    [[nodiscard]] const Argument* argument(Symbol name) const;

    /**
     * @brief Check if the instruction has an argument with the specified name.
     *
     * @param name The name of the argument.
     * @return `true` if it has an argument with the specified name, `false` otherwise.
     */
    [[nodiscard]] bool has_argument(Symbol name) const {return argument(name) != nullptr;}

    /**
     * @brief Compare two addressing modes based on their priority.
     *
     * @param other The other addressing mode to compare with.
     * @return `true` if this addressing mode has a lower priority than the other, `false` otherwise.
     */
    bool operator<(const AddressingMode& other) const {return priority < other.priority;}

    /// @brief The priority of the addressing mode, which determines the order in which addressing modes are considered.
    size_t priority = std::numeric_limits<size_t>::max();

    /// @brief Indicates whether the addressing mode uses the program counter (PC) in its encoding.
    bool uses_pc = false;

    /// @brief The notations that define the syntax of instructions with this addressing mode.
    std::vector<Notation> notations;

    /// @brief The arguments of the addressing mode.
    std::unordered_map<Symbol, std::unique_ptr<Argument>> arguments;

    /// @brief The encoding of the addressing mode.
    Body encoding;

    /**
     * @brief Add a notation to the addressing mode.
     *
     * @param notation The notation to add.
     */
    void add_notation(Notation notation) {notations.emplace_back(std::move(notation));}
};

#endif // HAD_XLR8_ADDRESSING_MODE_H
#undef IN_XLR8_ADDRESSING_MODE_H
