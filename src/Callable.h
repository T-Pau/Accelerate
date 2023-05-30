//
// Created by Dieter Baron on 29.05.23.
//

#ifndef CALLABLE_H
#define CALLABLE_H

#include "EvaluationContext.h"
#include "Expression.h"
#include "ParsedValue.h"
#include "Visibility.h"

class Callable {
  public:
    Callable(Token name, const std::shared_ptr<ParsedValue>& definition);
    [[nodiscard]] Symbol argument_name(size_t index) const {return argument_names[index];}
    [[nodiscard]] std::optional<Expression> default_argument(size_t index) const;
    [[nodiscard]] size_t maximum_arguments() const {return argument_names.size();}
    [[nodiscard]] size_t minimum_arguments() const {return argument_names.size() - default_arguments.size();}

    void parse_arguments(Tokenizer& tokenizer, bool comma_separated);

    Token name;
    Visibility visibility;
  protected:
    std::vector<Symbol> argument_names;
    std::vector<Expression> default_arguments;

    [[nodiscard]] EvaluationContext bind(const std::vector<Expression>& arguments) const;
    void serialize_callable(std::ostream& stream) const;

  private:
    static void initialize();

    static bool initialized;
    static Token token_arguments;
    static Token token_visibility;
};


#endif // CALLABLE_H
