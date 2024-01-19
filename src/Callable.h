/*
Callable.cc --

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

#ifndef CALLABLE_H
#define CALLABLE_H

#include "Entity.h"
#include "EvaluationContext.h"
#include "Expression.h"

class Callable: public Entity {
  public:
    class Arguments {
      public:
        Arguments() = default;
        Arguments(Tokenizer &tokenizer);

        void add(Symbol name, std::optional<Expression> default_argument);
        [[nodiscard]] bool empty() const {return names.empty();}
        [[nodiscard]] Symbol name(size_t index) const {return names[index];}
        [[nodiscard]] std::optional<Expression> default_argument(size_t index) const;
        [[nodiscard]] size_t maximum_arguments() const {return names.size();}
        [[nodiscard]] size_t minimum_arguments() const {return names.size() - default_arguments.size();}
        void serialize(std::ostream& stream) const;

        std::vector<Symbol> names;
        std::vector<Expression> default_arguments;
    };

    Callable(ObjectFile* owner, Token name, const std::shared_ptr<ParsedValue>& definition);
    Callable(ObjectFile* owner, Token name, Visibility visibility, bool default_only, Arguments arguments): Entity(owner, name, visibility, default_only), arguments(std::move(arguments)) {}
    [[nodiscard]] Symbol argument_name(size_t index) const {return arguments.name(index);}
    [[nodiscard]] std::optional<Expression> default_argument(size_t index) const {return arguments.default_argument(index);}

  protected:
    Arguments arguments;

    [[nodiscard]] std::shared_ptr<Environment> bind(const std::vector<Expression>& actual_arguments) const;
    void serialize_callable(std::ostream& stream) const;

  private:
    static void initialize();

    static bool initialized;
    static Token token_arguments;
};

std::ostream& operator<<(std::ostream& stream, const Callable::Arguments& arguments);

#endif // CALLABLE_H
