//
// Created by Dieter Baron on 30.12.22.
//

#include <iostream>

#include "CPUParser.h"
#include "Exception.h"
#include "Assembler.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " cpu-definition source" << std::endl;
        exit(1);
    }

    try {
        auto parser = CPUParser();
        auto cpu = parser.parse(argv[1]);

        auto assembler = Assembler(cpu);
        auto source = assembler.parse(argv[2]);

        std::cout << source;
    }
    catch (std::exception &ex) {
        std::cerr << "Parser error: " << ex.what() << std::endl;
    }
}
