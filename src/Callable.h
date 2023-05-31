//
// Created by Dieter Baron on 29.05.23.
//

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

    Callable(Token name, const std::shared_ptr<ParsedValue>& definition);
    Callable(Token name, Visibility visibility, Arguments arguments): Entity(name, visibility), arguments(std::move(arguments)) {}
    [[nodiscard]] Symbol argument_name(size_t index) const {return arguments.name(index);}
    [[nodiscard]] std::optional<Expression> default_argument(size_t index) const {return arguments.default_argument(index);}

  protected:
    Arguments arguments;

    [[nodiscard]] EvaluationContext bind(const std::vector<Expression>& actual_arguments) const;
    void serialize_callable(std::ostream& stream) const;

  private:
    static void initialize();

    static bool initialized;
    static Token token_arguments;
};

std::ostream& operator<<(std::ostream& stream, const Callable::Arguments& arguments);

#endif // CALLABLE_H
