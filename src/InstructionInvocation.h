#ifdef IN_XLR8_INSTRUCTION_INVOCATION_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_INSTRUCTION_INVOCATION_H
#ifndef HAD_XLR8_INSTRUCTION_INVOCATION_H
#define HAD_XLR8_INSTRUCTION_INVOCATION_H

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

#include <functional>
#include <vector>

#include <tpau-cpp-kernal/Symbol.h>

#include "AddressingMode.h"
#include "Body/IfBody.h"
#include "Node.h"
#include "Scope.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents an invocation of an instruction.
 *
 * It can produce the Body encoding the invocation.
 */
class InstructionInvocation {
  public:
    /**
     * @brief Construct an InstructionInvocation.
     *
     * @param cpu The CPU for which the instruction is invoked.
     * @param name The name of the instruction.
     * @param nodes The parsed nodes representing the arguments of the invocation.
     * @param containing_scope The scope containing the invocation.
     */
    InstructionInvocation(const CPU*, Token name, const std::vector<std::shared_ptr<Node>>& nodes, const std::shared_ptr<Scope>& containing_scope);

    /**
     * @brief Check if the instruction invocation uses the program counter (PC)..
     *
     * @return `true` if the instruction invocation uses the PC, `false` otherwise.
     */
    [[nodiscard]] bool uses_pc() { return uses_pc_; }

    /**
     * @brief Add the program counter (PC) to the instruction invocation.
     *
     * @param pc The PC constant to add.
     */
    void add_pc(Expression pc) { this->pc = std::move(pc); }

    /**
     * @brief Encode the instruction invocation.
     *
     * @return The encoded body of the instruction invocation.
     */
    [[nodiscard]] Body encode();

    /// @brief The location of the invocation in the source code.
    Location location;

  private:
    class Argument {
      public:
        Argument(const AddressingMode::Argument* definition, Node* node = nullptr);

        std::optional<Expression> constraint_expression(const Location& location, Symbol name) const;

        std::optional<bool> valid;
        std::optional<Value> known_value;
        const AddressingMode::Argument* definition;
    };

    class Variant {
      public:
        Variant(uint64_t opcode, const AddressingMode& addressing_mode, const AddressingMode::Notation& notation, const std::vector<std::shared_ptr<Node>>& nodes);

        /**
         * @brief Check if the instruction variant is valid.
         *
         * It is considered invalid if any of the arguments do not meet their type constraints.
         *
         * @return `true` if the variant is valid, `false` otherwise.
         */
        [[nodiscard]] bool valid() const { return valid_; }

        /**
         * @brief Encode the instruction variant.
         *
         * @param containing_scope The scope containing the instruction invocation.
         * @return A pair containing an optional constraint expression and the encoded body.
         */
        [[nodiscard]] std::pair<std::optional<Expression>, Body> encode(const std::shared_ptr<Scope>& containing_scope) const;

        /// @brief The opcode of the instruction variant.
        uint64_t opcode;

        /// @brief The addressing mode of the instruction variant.
        std::reference_wrapper<const AddressingMode> addressing_mode;

        /// @brief The notation of the instruction variant.
        std::reference_wrapper<const AddressingMode::Notation> notation;

        /// @brief The parsed arguments of the instruction invocation.
        std::reference_wrapper<const std::vector<std::shared_ptr<Node>>> nodes;

        /// @brief The arguments of the invocation, indexed by name
        std::unordered_map<Symbol, Argument> arguments;

        /// @brief The names of arguments with unknown values, indexed by their position in the argument list.
        std::unordered_map<size_t, Symbol> unknown_argument_names;

        /**
         * @brief The arguments that have been renamed due to different names in other variants.
         *
         * This is filled in by `InstructionInvocation::compute_argument_names()`.
         *
         * The key is the original name, and the value is renamed name.
         */
        std::unordered_map<Symbol, Symbol> argument_aliases;

        /// @brief If the variant is valid.
        bool valid_{true};
    };

    /**
     * @brief Compute the names of the arguments in the instruction invocation.
     *
     * This method populates the `argument_names` vector. If all variants have the same argument name at a position, it will get that name, otherwise it will get an internal name.
     */
    void compute_argument_names();

    /// @brief The variants of the instruction invocation.
    std::vector<Variant> variants;

    /// @brief The parsed invocation line.
    std::vector<std::shared_ptr<Node>> nodes;

    /// @brief The scope containing the instruction invocation.
    std::shared_ptr<Scope> containing_scope;

    /// @brief The scope with the arguments of the instruction invocation.
    std::shared_ptr<Scope> scope;

    /**
     * @brief The names of the arguments in the instruction invocation.
     *
     * If an argument has a known value, its name in this list is the empty symbol.
     */
    std::vector<Symbol> argument_names;

    /// @brief The arguments in the instruction invocation.
    std::vector<Expression> arguments;

    /// @brief Whether the invocation has arguments with unknown values.
    bool has_unknown_arguments{false};

    /// @brief Whether the invocation uses the program counter (PC).
    bool uses_pc_{false};

    std::optional<Expression> pc;
};

#endif // HAD_XLR8_INSTRUCTION_INVOCATION_H
#undef IN_XLR8_INSTRUCTION_INVOCATION_H
