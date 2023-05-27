//
// Created by Dieter Baron on 27.05.23.
//

#include "Visibility.h"

std::ostream& operator<<(std::ostream& stream, Visibility visibility) {
    switch  (visibility) {
        case Visibility::SCOPE:
            stream << "none";
            break;
        case Visibility::LOCAL:
            stream << "local";
            break;
        case Visibility::GLOBAL:
            stream << "global";
            break;
    }
    return stream;
}
