#ifdef IN_XLR8_NODE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_NODE_H
#ifndef HAD_XLR8_NODE_H
#define HAD_XLR8_NODE_H

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

#include <tpau-cpp-kernal/Location.h>

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a node in instruction notation. It can be an expression, keyword, or punctuation.
 */
class Node {
public:
    /**
     * @brief The type of the node.
     */
    enum Type {
        /// @brief The node is an expression, corresponding to an argument of the instruction.
        EXPRESSION,

        /// @brief The node is a keyword, corresponding to a reserved word in the instruction notation.
        KEYWORD,

        /// @brief The node is a punctuation, corresponding to a punctuation symbol in the instruction notation.
        PUNCTUATION
    };

    virtual ~Node() = default;

    /**
     * @brief Get the type of the node.
     *
     * Subclasses must implement this method.
     *
     * @return The type of the node.
     */
    [[nodiscard]] virtual Type type() const = 0;

    /**
     * @brief Get the location of the node in the source code.
     *
     * Subclasses must implement this method.
     *
     * @return The location of the node.
     */
    [[nodiscard]] virtual const Location& get_location() const = 0;
};

#endif // HAD_XLR8_NODE_H
#undef IN_XLR8_NODE_H
