#ifdef IN_XLR8_CALLABLE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_CALLABLE_H
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

#include "Entity/ScopeEntity.h"
#include "EvaluationContext.h"
#include "Expression/Expression.h"

/**
 * @brief Represents a callable entity that can be invoked with arguments: a macro or function.
 */
class Callable : public ScopeEntity {
  public:
    /**
     * @brief Represents the argument definitions of a callable entity.
     *
     * Each argument has a name. If the argument is optional, it also has a default value. No non-optional arguments can follow optional arguments.
     */

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
    Callable(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> parent_scope, bool default_only, Arguments arguments);

    /**
     * @brief Get the name of the argument at a given index.
     *
     * @param index The index of the argument.
     * @return The name of the argument.
     */
    [[nodiscard]] Symbol argument_name(size_t index) const { return arguments.name(index); }

    /**
     * @brief Get the default value of the argument at a given index.
     *
     * @param index The index of the argument.
     * @return The default value of the argument, if any.
     */
    [[nodiscard]] std::optional<Expression> default_argument(size_t index) const { return arguments.default_argument(index); }

    /**
     * @brief Get the list of argument names.
     *
     * @return The list of argument names.
     */
    [[nodiscard]] const std::vector<Symbol>& argument_names() const { return arguments.argument_names(); }

  protected:
    /// @brief The argument definitions.
    Arguments arguments;


    [[nodiscard]] EvaluationContext evaluation_context(EvaluationResult& result) override;

    void serialize_callable(std::ostream& stream) const;

  private:
};

#endif // HAD_XLR8_CALLABLE_H
#undef IN_XLR8_CALLABLE_H
