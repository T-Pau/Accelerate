//
// Created by Dieter Baron on 29.05.23.
//

#include "Function.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

#define DEFINITION "definition"

bool Function::initialized = false;
Token Function::token_definition;

void Function::initialize() {
    if (!initialized) {
        initialized = true;
        token_definition = Token(Token::NAME, DEFINITION);
    }
}

Function::Function(Token name, const std::shared_ptr<ParsedValue>& definition_) : Callable(name, definition_) {
    auto parameters = definition_->as_dictionary();

    auto tokenizer = SequenceTokenizer((*parameters)[token_definition]->as_scalar()->tokens);
    definition = Expression(tokenizer);
    if (!tokenizer.ended()) {
        throw ParseException(tokenizer.current_location(), "invalid definition");
    }
}


std::ostream& operator<<(std::ostream& stream, const Function& function) {
    function.serialize(stream);
    return stream;
}


Expression Function::call(const std::vector<Expression>& arguments) {
    return definition.evaluated(bind(arguments)).value_or(definition);
}


void Function::serialize(std::ostream& stream) const {
    stream << ".function " << name.as_string() << " {" << std::endl;
    serialize_callable(stream);
    stream << "    " DEFINITION ": " << definition << std::endl;
    stream << "}" << std::endl;
}
