#ifdef IN_XLR8_ADDRESS_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_ADDRESS_H
#ifndef HAD_XLR8_ADDRESS_H
#define HAD_XLR8_ADDRESS_H

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

#include <iostream>

#include "Expression/Expression.h"
#include "Tokenizer.h"

class Scope;

/**
 * Represents an address, consisting of a bank and an address within that bank. The bank defaults to 0.
 */
class Address {
  public:
    /**
     * Constructs an address with the given bank and address.
     *
     * @param bank The bank of the address.
     * @param address The address within the bank.
     */
    Address(uint64_t bank, uint64_t address) : bank_component(bank_name, bank), address_component(address_name, address) {}

    /**
     * Constructs an address with the given address and a default bank of 0.
     *
     * @param address The address within the bank.
     */
    explicit Address(uint64_t address) : bank_component(bank_name, 0), address_component(address_name, address) {}

    /**
     * Constructs an address with the given address.
     *
     * @param address The expression representing the address within the bank.
     */
    explicit Address(Expression address) : bank_component(bank_name, 0), address_component(address_name, std::move(address)) { evaluate_process(); }

    /**
     * Constructs an address with the given bank and address.
     *
     * @param bank The expression representing the bank of the address.
     * @param address The expression representing the address within the bank.
     */
    Address(Expression bank, Expression address) : bank_component(bank_name, std::move(bank)), address_component(address_name, std::move(address)) { evaluate_process(); }

    /**
     * Constructs an address by parsing an expression from a tokenizer.
     *
     * @param tokenizer The tokenizer to parse the expression from.
     * @param scope The scope to evaluate the expression in.
     */
    explicit Address(Tokenizer& tokenizer);

    /**
     * Serializes the address to a stream. The format is `address` if the bank is 0, and `bank:address` otherwise.
     */
    void serialize(std::ostream& stream) const;

    /**
     * Check if the bank is known.
     *
     * @return `true` if the bank is known, `false` otherwise.
     */
    [[nodiscard]] bool has_bank() const { return bank_component.has_value(); }

    /**
     * Check if the address within the bank is known.
     *
     * @return `true` if the address within the bank is known, `false` otherwise.
     */
    [[nodiscard]] bool has_address() const { return address_component.has_value(); }

    /**
     * @brief Get the bank of the address.
     *
     * @return The bank of the address, if known.
     */
    [[nodiscard]] std::optional<uint64_t> bank() const { return bank_component.value(); }

    /**
     * @brief Get the minimum possible value of the address within the bank.
     *
     * @return The minimum possible value of the address within the bank, if known.
     */
    [[nodiscard]] std::optional<uint64_t> address_minimum() const { return address_component.minimum(); }

    /**
     * @brief Get the maximum possible value of the address within the bank.
     *
     * @return The maximum possible value of the address within the bank, if known.
     */
    [[nodiscard]] std::optional<uint64_t> address_maximum() const { return address_component.maximum(); }

    /**
     * @brief Get the minimum possible value of the bank.
     *
     * @return The minimum possible value of the bank, if known.
     */
    [[nodiscard]] std::optional<uint64_t> bank_minimum() const { return bank_component.minimum(); }

    /**
     * @brief Get the maximum possible value of the bank.
     *
     * @return The maximum possible value of the bank, if known.
     */
    [[nodiscard]] std::optional<uint64_t> bank_maximum() const { return bank_component.maximum(); }

    /**
     * @brief Get the address within the bank.
     *
     * @return The address within the bank, if known.
     */
    [[nodiscard]] std::optional<uint64_t> address() const { return address_component.value(); }

    /**
     * @brief Resolve names in the expressions.
     *
     * @param scope The scope to resolve names in.
     * @param containing_entity The entity containing the address.
     */
    void resolve(Scope* scope, Entity* containing_entity);

    /**
     * @brief Evaluate the expressions.
     *
     * @param context The evaluation context.
     */
    void evaluate(const EvaluationContext& context);

    bool operator==(const Address& other) const;
    bool operator<(const Address& other) const;

  private:
    class Component {
      public:
        explicit Component(Symbol name, Expression expression) : name(std::move(name)), component_value(std::move(expression)) {}

        explicit Component(Symbol name, uint64_t value) : name(std::move(name)), component_value(value) {}

        [[nodiscard]] bool has_value() const;
        [[nodiscard]] std::optional<uint64_t> value() const;

        void resolve(Scope* scope, Entity* containing_entity);
        void evaluate(const EvaluationContext& context);
        void evaluate_process();

        [[nodiscard]] std::optional<bool> is_equal(const Component& other) const;
        [[nodiscard]] std::optional<bool> is_not_equal(const Component& other) const;
        [[nodiscard]] std::optional<bool> is_less_than(const Component& other) const;

        [[nodiscard]] std::optional<uint64_t> minimum() const;
        [[nodiscard]] std::optional<uint64_t> maximum() const;

        void serialize(std::ostream& stream, bool output_if_zero = true) const;

      private:
        Symbol name;
        std::variant<Expression, uint64_t> component_value;
    };

    /**
     * Writes an address component to a stream.
     */
    friend std::ostream& operator<<(std::ostream& stream, Component component);

    /**
     * @brief Set values from expressions.
     */
    void evaluate_process();

    /// The bank of the address.
    Component bank_component;

    /// The address within the bank.
    Component address_component;

    static Symbol bank_name;
    static Symbol address_name;
};

/**
 * Writes an address to a stream. The format is `address` if the bank is 0, and `bank:address` otherwise.
 */
std::ostream& operator<<(std::ostream& stream, Address address);

bool operator<(const std::optional<Address>& a, const std::optional<Address>& b);

#endif // HAD_XLR8_ADDRESS_H
#undef IN_XLR8_ADDRESS_H
