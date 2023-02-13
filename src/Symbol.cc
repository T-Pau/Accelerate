//
// Created by Dieter Baron on 24.01.23.
//

#include "Symbol.h"

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Symbol>& object) {
    object->serialize(stream);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const Symbol& object) {
    object.serialize(stream);
    return stream;
}

void Symbol::serialize(std::ostream &stream) const {
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

void Symbol::append(const ExpressionList &list) {
    data.append(list);
    size = data.byte_size();
}

void Symbol::append(const std::shared_ptr<ExpressionNode> &expression) {
    data.append(expression);
    size = data.byte_size();
}
