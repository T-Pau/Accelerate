//
// Created by Dieter Baron on 29.05.23.
//

#include "Function.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

bool Function::initialized = false;
Token Function::token_value;

void Function::initialize() {
    if (!initialized) {
        initialized = true;
        token_value = Token(Token::NAME, "value");
    }
}

Function::Function(Token name, const std::shared_ptr<ParsedValue>& definition) : Callable(name, definition) {
    auto parameters = definition->as_dictionary();

    auto tokenizer = SequenceTokenizer((*parameters)[token_value]->as_scalar()->tokens);
    value = Expression(tokenizer);
    if (!tokenizer.ended()) {
        throw ParseException(tokenizer.current_location(), "invalid value");
    }
}


std::ostream& operator<<(std::ostream& stream, const Function& function) {
    function.serialize(stream);
    return stream;
}


Expression Function::call(const std::vector<Expression>& arguments) {
    return value.evaluated(bind(arguments)).value_or(value);
}


void Function::serialize(std::ostream& stream) const {
    stream << ".function " << name.as_string() << "{" << std::endl;
    serialize_callable(stream);
    stream << "    value: " << value << std::endl;
    stream << "}" << std::endl;
}
