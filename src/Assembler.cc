//
// Created by Dieter Baron on 30.12.22.
//

#include <iostream>

#include "FileReader.h"
#include "Tokenizer.h"
#include "CPUParser.h"
#include "Exception.h"

int main(int argc, char *argv[]) {
    auto reader = FileReader();

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " cpu-definition" << std::endl;
    }

    auto parser = CPUParser();
    try {
        auto cpu = parser.parse(argv[1], reader);
    }
    catch (Exception &ex) {
        std::cerr << "Parser error: " << ex.what() << std::endl;
    }
}
