//
// Created by Dieter Baron on 17.03.23.
//

#ifndef TARGET_H
#define TARGET_H

#include <vector>

#include "CPU.h"
#include "MemoryMap.h"
#include "OutputElement.h"

class Target {
public:

    void add_output_element(OutputElement element) {output_elements.emplace_back(std::move(element));}

    CPU cpu;
    MemoryMap map;

    std::vector<OutputElement> output_elements;
};


#endif // TARGET_H
