//
// Created by Dieter Baron on 27.05.23.
//

#ifndef VISIBILITY_H
#define VISIBILITY_H

#include <iostream>

enum class Visibility {
    SCOPE,
    LOCAL,
    GLOBAL
};

std::ostream& operator<<(std::ostream& stream, Visibility visibility);

#endif // VISIBILITY_H
