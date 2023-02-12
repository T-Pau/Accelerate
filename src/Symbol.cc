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
    stream << ".object " << name.as_string();
}
