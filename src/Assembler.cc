//
// Created by Dieter Baron on 30.12.22.
//

#include <iostream>

#include "FileReader.h"
#include "Tokenizer.h"
#include "CPUParser.h"
#include "Exception.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " cpu-definition source" << std::endl;
    }

    try {
        auto parser = CPUParser();
        auto cpu = parser.parse(argv[1]);

        
    }
    catch (Exception &ex) {
        std::cerr << "Parser error: " << ex.what() << std::endl;
    }
}
