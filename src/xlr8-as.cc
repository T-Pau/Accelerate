//
// Created by Dieter Baron on 30.12.22.
//

#include <iostream>
#include <fstream>
#include <vector>

#include "Assembler.h"
#include "Commandline.h"
#include "CPUParser.h"

int main(int argc, char *argv[]) {
    std::vector<Commandline::Option> options = {
            Commandline::Option("cpu", "file", "read CPU definition from FILE"),
            Commandline::Option("output", 'o', "file", "write output to FILE"),
            Commandline::Option("depfile", 'M', "file", "write gcc-style dependency file to FILE")
    };

    auto commandline = Commandline(options, "source.s", "", "", "");

    auto arguments = commandline.parse(argc, argv);

    try {
        auto cpu_file = arguments.find_first("cpu");
        if (!cpu_file.has_value()) {
            std::cerr << argv[0] << ": missing --cpu option" << std::endl;
            exit(1);
        }
        auto parser = CPUParser();
        auto cpu = parser.parse(cpu_file.value());

        auto assembler = Assembler(cpu);
        auto source = assembler.parse(arguments.arguments[0]);

        if (FileReader::global.had_error()) {
            exit(1);
        }

        auto output_file = arguments.find_first("output");
        if (!output_file.has_value()) {
            output_file = std::filesystem::path(arguments.arguments[0]).stem().filename().string() + ".o";
        }

        {
            auto stream = std::ofstream(output_file.value());
            stream << source;
        }

        auto depfile = arguments.find_first("depfile");
        if (depfile.has_value()) {
            auto stream = std::ofstream(depfile.value());

            stream << output_file.value() << ":";
            for (const auto& file_name: FileReader::global.file_names()) {
                stream << " " << file_name;
            }
            stream << std::endl;
        }
    }
    catch (std::exception &ex) {
        std::cerr << "Parser error: " << ex.what() << std::endl;
    }
}
