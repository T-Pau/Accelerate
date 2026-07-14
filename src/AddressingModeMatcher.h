#ifdef IN_XLR8_ADDRESSING_MODE_MATCHER_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_ADDRESSING_MODE_MATCHER_H
#ifndef HAD_XLR8_ADDRESSING_MODE_MATCHER_H
#define HAD_XLR8_ADDRESSING_MODE_MATCHER_H

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
#include <unordered_set>

#include <tpau-cpp-kernal/Symbol.h>

#include "Node.h"
#include "AddressingMode.h"

using namespace tpau::cpp_kernal;

class AddressingModeMatcherElement {
public:
    enum Type {
        PUNCTUATION,
        KEYWORD,
        INTEGER
    };

    AddressingModeMatcherElement(Type type, Symbol symbol): type(type), symbol(symbol) {}
    AddressingModeMatcherElement(): type(INTEGER) {}
    explicit AddressingModeMatcherElement(Node* node);

    bool operator==(const AddressingModeMatcherElement& other) const;

    Type type = INTEGER;
    Symbol symbol;

    static std::vector<AddressingModeMatcherElement> elements_for(const AddressingMode::Notation::Element& element, const std::unordered_map<Symbol, std::unique_ptr<AddressingMode::Argument>>& arguments);
};


template<>
struct std::hash<AddressingModeMatcherElement>
{
    std::size_t operator()(AddressingModeMatcherElement const& element) const noexcept {
        size_t h1 = std::hash<size_t>{}(static_cast<size_t>(element.type));
        size_t h2 = 0;
        switch (element.type) {
            case AddressingModeMatcherElement::INTEGER:
                break;

            case AddressingModeMatcherElement::KEYWORD:
            case AddressingModeMatcherElement::PUNCTUATION:
                h2 = std::hash<Symbol>{}(element.symbol);
                break;
        }
        return h1 ^ (h2 << 1);
    }
};

/// @brief Represents a match for the given line.
class AddressingModeMatcherResult {
public:
    /**
     * @brief Initialize a match result with the given addressing mode and notation index.
     *
     * @param addressing_mode The addressing mode that matched.
     * @param notation_index The index of the addressing mode's notation that matched.
     */
    AddressingModeMatcherResult(Symbol addressing_mode, size_t notation_index): addressing_mode(addressing_mode), notation_index(notation_index) {}

    /**
     * @brief Check if two match results are equal.
     *
     * @param other The other match result to compare with.
     * @return `true` if the match results are equal, `false` otherwise.
     */
    bool operator==(const AddressingModeMatcherResult& other) const {return addressing_mode == other.addressing_mode && notation_index == other.notation_index;}

    /// @brief The addressing mode that matched.
    Symbol addressing_mode;

    /// @brief The index of the addressing mode's notation that matched.
    size_t notation_index;
};


template<>
struct std::hash<AddressingModeMatcherResult>
{
    std::size_t operator()(AddressingModeMatcherResult const& result) const noexcept {
        return std::hash<Symbol>{}(result.addressing_mode) ^ (std::hash<size_t>{}(result.notation_index) << 1);
    }
};


/**
 * @brief Determines which addressing modes match a given line in the source code.
 *
 * It does not take into account the constraints of the argument types, only if the syntax matches.
 *
 * It is implemented via a deterministic finite automaton (DFA) that is built from all notations of all addressing modes.
 *
 * When matched against a line, it returns all addressing modes that match the line, along with the index of the notation that matched.
 */
class AddressingModeMatcher {
public:
    /**
     * @brief Match a line in the source code against all known addressing modes.
     *
     * @param nodes The nodes representing the line in the source code.
     * @return A set of match results for the line.
     */
    [[nodiscard]] std::unordered_set<AddressingModeMatcherResult> match(const std::vector<std::shared_ptr<Node>>& nodes) const;

    /**
     * @brief Add a notation for an addressing mode to the matcher.
     *
     * @param addressing_mode The name of the addressing mode.
     * @param notation_index The index in the addressing mode's notations.
     * @param notation The notation to add.
     * @param arguments The arguments for the addressing mode.
     */
    void add_notation(Symbol addressing_mode, size_t notation_index, const AddressingMode::Notation& notation, const std::unordered_map<Symbol, std::unique_ptr<AddressingMode::Argument>>& arguments);

private:
    /// @brief Represents a node in the matcher DFA.
    class MatcherNode {
    public:
        /**
         * @brief Add a notation to the matcher.
         *
         * This method recursively adds the elements of the notation to the matcher, creating new nodes as necessary.
         *
         * @param result The match result associated with the notation.
         * @param current The current element in the notation being processed.
         * @param end The end iterator of the notation's elements.
         * @param arguments The arguments for the addressing mode.
         */
        void add_notation(const AddressingModeMatcherResult& result, std::vector<AddressingMode::Notation::Element>::const_iterator current, std::vector<AddressingMode::Notation::Element>::const_iterator end, const std::unordered_map<Symbol, std::unique_ptr<AddressingMode::Argument>>& arguments);

        /// @brief The set of match results associated with this node in the matcher DFA.
        std::unordered_set<AddressingModeMatcherResult> results;

        /// @brief The next node in the matcher DFA for each next node from the source code.
        std::unordered_map<AddressingModeMatcherElement, std::unique_ptr<MatcherNode>> next;
    };

    /// @brief The start node of the matcher DFA.
    MatcherNode start;
};


#endif // HAD_XLR8_ADDRESSING_MODE_MATCHER_H
#undef IN_XLR8_ADDRESSING_MODE_MATCHER_H
