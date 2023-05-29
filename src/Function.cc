//
// Created by Dieter Baron on 29.05.23.
//

#include "Function.h"

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
