//
// Created by Dieter Baron on 27.05.23.
//

#include "Address.h"

std::ostream& operator<<(std::ostream& stream, Address address) {
    address.serialize(stream);
    return stream;
}

void Address::serialize(std::ostream &stream) const {
    if (bank > 0) {
        stream << bank << ":";
    }
    stream << address;
}
