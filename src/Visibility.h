#ifdef IN_XLR8_VISIBILITY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_VISIBILITY_H
#ifndef HAD_XLR8_VISIBILITY_H
#define HAD_XLR8_VISIBILITY_H

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

#include <format>
#include <iostream>
#include <optional>

#include "Token.h"

// Keep in sync with VisibilityHelper::names.
/**
 * @brief Represents the visibility of a declaration.
 */
enum class Visibility {
    /**
     * This visibility is used for constants that are only visible in the current body.
     *
     * It is used for parameters to macros and functions, and for the loop variable in `.repeat`.
     *
     * This scope cannot be selected via a visibility directive.
     */
    ARGUMENT,

    /**
     * This visibility is used for constants that are only visible within the current entity.
     *
     * It is the default visibility for names defined in an entity.
     */
    ENTITY,

    /**
     * This visibility is used for entities that are only visible within the current file.
     *
     * This visibility is selected via the `.local` visibility directive.
     */
    FILE,

    /**
     * This visibility is used for entities that are only visible within the current module.
     *
     * This visibility is selected via the `.private` visibility directive.
     */
    PRIVATE,

    /**
     * This visibility is used for entities that are visible outside the current module.
     *
     * This visibility is selected via the `.public` visibility directive.
     *
     * The main entry point of a program (usually `start`) must have this visibility.
     */
    PUBLIC,
};

/**
 * @brief Helper class for working with Visibility.
 */
class VisibilityHelper {
  public:
    /**
     * @brief Converts a token to a Visibility value.
     *
     * It supports both the visibility directive tokens (e.g., `.public`) and the visibility name tokens (e.g., `public`).
     *
     * If the token does not represent a visibility, returns {}.
     *
     * @param token The token to convert.
     * @return The corresponding Visibility value, or {} if the token does not represent a visibility.
     */
    static std::optional<Visibility> from_token(const Token& token);

    /**
     * @brief Converts a Visibility value to a token.
     *
     * @param visibility The visibility value to convert.
     * @param directive If true, returns the corresponding directive token (e.g., `.public`).
     *                  If false, returns the corresponding name token (e.g., `public`).
     * @return The corresponding token.
     */
    static Token to_token(Visibility visibility, bool directive = true);

    /**
     * @brief Get the name of a visibility.
     *
     * @param visibility The visibility to convert.
     * @return The corresponding name.
     */
    static Symbol name(Visibility visibility) { return names[static_cast<size_t>(visibility)]; }

    /// @brief The token for directive for the `FILE` visibility (`.local`).
    static const Token token_file_directive;

    /// @brief The token for name for the `FILE` visibility (`local`).
    static const Token token_file_name;

    /// @brief The token for directive for the `PRIVATE` visibility (`.private`).
    static const Token token_private_directive;

    /// @brief The token for name for the `PRIVATE` visibility (`private`).
    static const Token token_private_name;

    /// @brief The token for directive for the `PUBLIC` visibility (`.public`).
    static const Token token_public_directive;

    /// @brief The token for name for the `PUBLIC` visibility (`public`).
    static const Token token_public_name;

    /// @brief The name of the `ARGUMENT` visibility (`argument`).
    static const char argument_name[];

    /// @brief The name of the `ENTITY` scope type (`entity`).
    static const char entity_name[];

    /// @brief The name of the `FILE` scope type (`file`).
    static const char file_name[];

    /// @brief The name for the `FILE` visibility and scope type (`local`).
    static const char file_literal[];

    /// @brief The name for the `PRIVATE` visibility and scope type (`private`).
    static const char private_literal[];

    /// @brief The name for the `PUBLIC` visibility and scope type (`public`).
    static const char public_literal[];

  private:
    /// @brief The names of the visibilities.
    static Symbol names[];
};

/**
 * @brief Outputs the directive for a Visibility value to a stream.
 *
 * @param stream The stream to output to.
 * @param visibility The Visibility value to output.
 * @return The stream.
 */
std::ostream& operator<<(std::ostream& stream, Visibility visibility);

template <> struct std::formatter<Visibility> : std::formatter<Symbol> {
    auto format(const Visibility& visibility, format_context& ctx) const { return std::formatter<Symbol>::format(VisibilityHelper::name(visibility), ctx); }
};

#endif // HAD_XLR8_VISIBILITY_H
#undef IN_XLR8_VISIBILITY_H
