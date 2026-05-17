/*
Symbol.h -- Map Strings to Integers

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

#ifndef SYMBOL_H
#define SYMBOL_H

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable;

/// @brief Class for hashing string pointers.
struct StringPtrHash {
    auto operator()(const std::string* string) const noexcept {
        return std::hash<std::string>{}(*string);
    }
};

/// @brief Class for comparing string pointers.
struct StringPtrEqual {
    auto operator()(const std::string* a, const std::string* b) const {
        return *a == *b;
    }
};

/// @brief A Symbol represents a string in a way that allows comparison in constant time.
class Symbol {
public:
    /// @brief Create an empty symbol.
    Symbol() = default;

    /**
     * Create a symbol from a string.
     * 
     * @param name The string to create the symbol from.
     * @return The created symbol.
     */
    explicit Symbol(const std::string& name);

    /**
     * Assign a symbol the symbol corresponding to a string.
     * 
     * @param name The string to create the symbol from.
     * @return The created symbol.
     */
    Symbol& operator=(const std::string& name);

    /**
     * Get the string represented by the symbol.
     * 
     * @return The string represented by the symbol.
     */
    [[nodiscard]] const std::string& str() const {return *id;}

    /**
     * Get the C string represented by the symbol.
     * 
     * @return The C string represented by the symbol.
     */
    [[nodiscard]] const char* c_str() const {return id->c_str();}

    /** 
     * Check if the symbol is empty.
     * 
     * @return `true` if the symbol is empty, `false` otherwise.
     */
    [[nodiscard]] bool empty() const {return id==&empty_string;}

    /**
     * Check if two symbols are equal.
     * 
     * @param other The symbol to compare with.
     * @return `true` if the symbols are equal, `false` otherwise.
     */
    bool operator==(const Symbol& other) const {return id == other.id;}

    /**
     * Check if two symbols are not equal.
     * 
     * @param other The symbol to compare with.
     * @return `true` if the symbols are not equal, `false` otherwise.
     */
    bool operator!=(const Symbol& other) const {return id != other.id;}

    /**
     * Check if this symbol is less than another symbol.
     * 
     * @param other The symbol to compare with.
     * @return `true` if this symbol is less than the other symbol, `false` otherwise.
     */
    bool operator<(const Symbol& other) const {return *id < *other.id;}

    /**
     * Check if this symbol is less than or equal to another symbol.
     * 
     * @param other The symbol to compare with.
     * @return `true` if this symbol is less than or equal to the other symbol, `false` otherwise.
     */

     bool operator<=(const Symbol& other) const {return *id <= *other.id;}
    /**
     * Check if this symbol is greater than another symbol.
     * 
     * @param other The symbol to compare with.
     * @return `true` if this symbol is greater than the other symbol, `false` otherwise.
     */

     bool operator>(const Symbol& other) const {return *id > *other.id;}
    /**
     * Check if this symbol is greater than or equal to another symbol.
     * 
     * @param other The symbol to compare with.
     * @return `true` if this symbol is greater than or equal to the other symbol, `false` otherwise.
     */

     bool operator>=(const Symbol& other) const {return *id >= *other.id;}
    /**
     * Check if the symbol is valid (not empty).
     * 
     * @return `true` if the symbol is valid, `false` otherwise.
     */
    operator bool() const {return !empty();} // NOLINT(*-explicit-constructor)

  private:
    /// @brief A table of all symbols, used to ensure that each symbol is only stored once.
    struct Table {
      public:
        const std::string* intern(const std::string& string);

      private:
        /// @brief This table maps raw pointers of interned strings to unique pointers of the same strings.
        std::unordered_map<const std::string*,std::unique_ptr<std::string>,StringPtrHash,StringPtrEqual> symbols;
    };

    /// @brief Pointer to the interned string represented by the symbol.
    const std::string* id{&empty_string};

    /// @brief The empty string, used for empty symbols.
    static const std::string empty_string;

    /// @brief Initialize the global symbol table.
    static void init_global();

    /**
     * The global symbol table.
     * 
     * It is a pointer to an object so we can force initialization before we use it in constructing symbols.
     */
    static Table *global;
};

template<>
struct std::hash<Symbol>
{
    std::size_t operator()(Symbol const& symbol) const noexcept {
        return std::hash<const char*>{}(symbol.c_str());
    }
};

/**
 * Output a symbol to a stream.
 * 
 * @param stream The stream to output to.
 * @param symbol The symbol to output.
 * @return The stream that was output to.
 */
std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);

#endif // SYMBOL_H
