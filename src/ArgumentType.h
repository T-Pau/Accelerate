#ifdef IN_XLR8_ARGUMENT_TYPE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_ARGUMENT_TYPE_H
#ifndef HAD_XLR8_ARGUMENT_TYPE_H
#define HAD_XLR8_ARGUMENT_TYPE_H

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

#include <unordered_map>

#include <tpau-cpp-kernal/Symbol.h>
#include <tpau-cpp-kernal/Value.h>

#include "IntegerEncoder.h"
#include "Expression/Expression.h"

using namespace tpau::cpp_kernal;

class ArgumentTypeEncoding;

/**
 * @brief Represents an argument type, which defines the valid values for an instruction's operands.
 */
class ArgumentType {
public:
    /// @brief The type of the argument type.
    enum Type {
        /// @brief The argument type accepts any value.
        ANY,

        /// @brief The argument type accepts values that can be encoded by a specific encoding.
        ENCODING,

        /// @brief The argument type accepts one of a fixed set of keywords.
        ENUM,

        /// @brief The argument type accepts one of a fixed set of values that are mapped to other values.
        MAP,

        /// @brief The argument type accepts values within a specific range.
        RANGE
    };

    /**
     * @brief Initialize an ArgumentType.
     *
     * @param name The name of the argument type.
     */
    explicit ArgumentType(Symbol name): name(name) {}

    virtual ~ArgumentType() = default;

    /**
     * @brief Get the type of the argument type.
     *
     * Subclasses must implement this method.
     *
     * @return The type of the argument type.
     */
    [[nodiscard]] virtual Type type() const = 0;

    /**
     * @brief Get the argument type as type T.
     *
     * @tparam T The type to cast to.
     * @return A pointer to the argument type as type T, or `nullptr` if it is not of type T.
     */
    template<typename T>
    [[nodiscard]] T* as() {return dynamic_cast<T*>(this);}

    /**
     * @brief Get the argument type as type T.
     *
     * @tparam T The type to cast to.
     * @return A pointer to the argument type as type T, or `nullptr` if it is not of type T.
     */
    template<typename T>
    [[nodiscard]] const T* as() const {return dynamic_cast<const T*>(this);}

    /**
     * @brief Check if the argument type is of type T.
     *
     * @tparam T The type to check against.
     * @return `true` if the argument type is of type T, `false` otherwise.
     */
    template<typename T>
    [[nodiscard]] bool is() const {return as<T>() != nullptr;}

    /**
     * @brief Check if an expression is valid for the argument type.
     *
     * The default implementation returns {}. Subclasses should override this method to implement specific validation logic.
     *
     * @param expression The expression to check.
     * @return `true` if the expression is known to be valid for the argument type, `false` if it is known to be invalid, {} if it is unknown.
     */
    [[nodiscard]] virtual std::optional<bool> is_valid(const Expression& expression) const {return {};}

    /**
     * @brief Get the constraint expression for the argument type.
     *
     * @param location The location of the argument in the source code.
     * @param name The name to use for the argument.
     * @return The constraint expression, or {} if there is no constraint.
     */
    [[nodiscard]] virtual std::optional<Expression> constraint_expression(const Location& location, Symbol name) const {return {};}

    /// @brief The name of the argument type.
    Symbol name;
};

/// @brief Represents an argument type that accepts any value.
class ArgumentTypeAny: public ArgumentType {
public:
    /**
     * @brief Initialize an ArgumentTypeAny.
     *
     * @param name The name of the argument type.
     */
    explicit ArgumentTypeAny(Symbol name): ArgumentType(name) {}

    [[nodiscard]] std::optional<bool> is_valid(const Expression& expression) const override {return true;}

    [[nodiscard]] Type type() const override {return ANY;}
};

/// @brief Represents an argument type that accepts values that can be encoded by a specific encoding.
class ArgumentTypeEncoding: public ArgumentType {
public:
    /**
     * @brief Initialize an ArgumentTypeEncoding.
     *
     * @param name The name of the argument type.
     * @param encoding The encoding used by the argument type.
     */
    explicit ArgumentTypeEncoding(Symbol name, const IntegerEncoder& encoding): ArgumentType(name), encoding(encoding) {}

    [[nodiscard]] Type type() const override {return ENCODING;}
    [[nodiscard]] std::optional<bool> is_valid(const Expression& expression) const override;
    [[nodiscard]] std::optional<Expression> constraint_expression(const Location& location, Symbol name) const override;

    // TODO: document
    [[nodiscard]] std::unique_ptr<ArgumentType> range_type(Symbol range_name) const;

    /// @brief The encoding used by the argument type.
    IntegerEncoder encoding;
};

/// @brief Represents an argument type that accepts one of a fixed set of keywords.
class ArgumentTypeEnum: public ArgumentType {
public:
    /**
     * @brief Initialize an ArgumentTypeEnum.
     *
     * @param name The name of the argument type.
     */
    explicit ArgumentTypeEnum(Symbol name): ArgumentType(name) {}


    [[nodiscard]] Type type() const override {return ENUM;}

    /**
     * @brief Check if the argument type has an entry with the specified name.
     *
     * @param name The name of the entry.
     * @return `true` if the entry exists, `false` otherwise.
     */
    [[nodiscard]] bool has_entry(Symbol name) const {return entries.contains(name);}

    /**
     * @brief Get the value of the entry with the specified name.
     *
     * @param name The name of the entry.
     * @return The value of the entry.
     */
    [[nodiscard]] Value entry(Symbol name) const;

    /// @brief The accepted keywords and their corresponding values.
    std::unordered_map<Symbol, Value> entries;
};



/// @brief Represents an argument type that accepts one of a fixed set of values that are mapped to other values.
class ArgumentTypeMap: public ArgumentType {
public:
    /**
     * @brief Initialize an ArgumentTypeMap.
     *
     * @param name The name of the argument type.
     */
    explicit ArgumentTypeMap(Symbol name): ArgumentType(name) {}

    [[nodiscard]] Type type() const override {return MAP;}

    [[nodiscard]] std::optional<bool> is_valid(const Expression& expression) const override;

    /**
     * @brief Check if the argument type has an entry for the specified value.
     *
     * @param value The value to check.
     * @return `true` if the entry exists, `false` otherwise.
     */
    [[nodiscard]] bool has_entry(const Value& value) const {return entries.contains(value);}

    /**
     * @brief Get the mapped value for the specified input value.
     *
     * @param value The input value.
     * @return The mapped value.
     */
    [[nodiscard]] Value entry(const Value& value) const;

    /// @brief The accepted input values and their corresponding mapped values.
    std::unordered_map<Value, Value> entries;
};



/// @brief Represents an argument type that accepts values within a specific range.
class ArgumentTypeRange: public ArgumentType {
public:
    /**
     * @brief Initialize an ArgumentTypeRange.
     *
     * @param name The name of the argument type.
     */
    explicit ArgumentTypeRange(Symbol name): ArgumentType(name) {}

    [[nodiscard]] Type type() const override {return RANGE;}

    [[nodiscard]] std::optional<bool> is_valid(const Expression& expression) const override;
    [[nodiscard]] std::optional<Expression> constraint_expression(const Location& location, Symbol name) const override;

    /// @brief The lower bound of the accepted range.
    Value lower_bound;

    /// @brief The upper bound of the accepted range.
    Value upper_bound;
};

#endif // HAD_XLR8_ARGUMENT_TYPE_H
#undef IN_XLR8_ARGUMENT_TYPE_H
