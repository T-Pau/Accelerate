//
// Created by Dieter Baron on 29.05.23.
//

#include "Macro.h"
#include "Exception.h"

std::ostream& operator<<(std::ostream& stream, const Macro& macro) {
    macro.serialize(stream);
    return stream;
}


void Macro::serialize(std::ostream& stream) const {
    stream << ".macro " << name.as_string() << " {" << std::endl;
    serialize_callable(stream);
    stream << "    body <" << std::endl;
    body.serialize(stream, "        ");
    stream << "    >" << std::endl;
    stream << "}" << std::endl;

}


Body Macro::expand(const std::vector<Expression>& arguments) const {
    // TODO: enclose in scope
    return body.evaluated(bind(arguments)).value_or(body);
}
