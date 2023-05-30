//
// Created by Dieter Baron on 29.05.23.
//

#include "Macro.h"
#include "Exception.h"

bool Macro::initialized = false;
Token Macro::token_body;

void Macro::initialize() {
    if (!initialized) {
        initialized = true;
        token_body = Token(Token::NAME, "body");
    }
}


Macro::Macro(Token name, const std::shared_ptr<ParsedValue>& definition) : Callable(name, definition) {
    auto parameters = definition->as_dictionary();

    body = (*parameters)[token_body]->as_body()->body;
}


std::ostream& operator<<(std::ostream& stream, const Macro& macro) {
    macro.serialize(stream);
    return stream;
}


Body Macro::expand(const std::vector<Expression>& arguments) const {
    // TODO: enclose in scope
    return body.evaluated(bind(arguments)).value_or(body);
}


void Macro::serialize(std::ostream& stream) const {
    stream << ".macro " << name.as_string() << " {" << std::endl;
    serialize_callable(stream);
    stream << "    body <" << std::endl;
    body.serialize(stream, "        ");
    stream << "    >" << std::endl;
    stream << "}" << std::endl;

}
