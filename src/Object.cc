//
// Created by Dieter Baron on 24.01.23.
//

#include "Object.h"

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Object>& object) {
    object->serialize(stream);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const Object& object) {
    object.serialize(stream);
    return stream;
}

void Object::serialize(std::ostream &stream) const {
    stream << ".object " << name.as_string() << " {" << std::endl;
    if (alignment > 0) {
        stream << "    alignment: " << alignment << std::endl;
    }
    stream << "    section: " << SymbolTable::global[section] << std::endl;
    stream << "    size: " << size << std::endl;
    // TODO: visibility
    if (!data.empty()) {
        stream << "    data: " << data << std::endl;
    }

    stream << "}" << std::endl;
}

void Object::append(const ExpressionList &list) {
    data.append(list);
    size = data.byte_size();
}

void Object::append(const std::shared_ptr<ExpressionNode> &expression) {
    data.append(expression);
    size = data.byte_size();
}
