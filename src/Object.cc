//
// Created by Dieter Baron on 24.01.23.
//

#include "Object.h"
#include "Exception.h"

std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Object>& object) {
    object->serialize(stream);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const Object& object) {
    object.serialize(stream);
    return stream;
}

std::ostream& operator<< (std::ostream& stream, Object::Visibility visibility) {
    switch  (visibility) {
        case Object::NONE:
            stream << "none";
            break;
        case Object::LOCAL:
            stream << "local";
            break;
        case Object::GLOBAL:
            stream << "global";
            break;
    }
    return stream;
}

void Object::serialize(std::ostream &stream) const {
    stream << ".object " << name.as_string() << " {" << std::endl;
    if (alignment > 0) {
        stream << "    alignment: " << alignment << std::endl;
    }
    stream << "    section: " << section << std::endl;
    stream << "    size: " << size << std::endl;
    stream << "    visibility: " << visibility << std::endl;
    if (!data.empty()) {
        stream << "    data: " << data << std::endl;
    }

    stream << "}" << std::endl;
}

void Object::append(const ExpressionList &list) {
    data.append(list);
    size = data.byte_size();
}

void Object::append(const std::shared_ptr<Expression> &expression) {
    data.append(expression);
    size = data.byte_size();
}

Object::Object(const ObjectFile *owner, const Object *object): owner(owner), section(object->section), visibility(object->visibility), name(object->name), size(object->size) {
    if (!object->is_reservation()) {
        append(object->data);
    }
}
