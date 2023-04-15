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
    Target() = default;
    explicit Target(Symbol name): name(name) {}

    static const Target& get(Symbol name);
    static const Target& get(const std::string& name) {return get(Symbol(name));}
    static const Target empty;

    [[nodiscard]] bool is_compatible_with(const Target& other) const; // this has everything from other

    void add_output_element(OutputElement element) {output_elements.emplace_back(std::move(element));}

    Symbol name;
    const CPU* cpu = &CPU::empty;
    MemoryMap map;

    std::vector<OutputElement> output_elements;

    std::string extension = "bin";

};

#endif // TARGET_H
