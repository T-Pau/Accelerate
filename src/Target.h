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
    Target() {}
    explicit Target(Symbol name): name(name) {}

    static const Target empty;

    void add_output_element(OutputElement element) {output_elements.emplace_back(std::move(element));}

    Symbol name;
    CPU cpu;
    MemoryMap map;

    std::vector<OutputElement> output_elements;

    std::string extension = "bin";

};

#endif // TARGET_H
