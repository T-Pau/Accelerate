/*
xlr8-ld.cc -- 

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <fstream>
#include <vector>

#include "Assembler.h"
#include "Commandline.h"
#include "CPUParser.h"
#include "ObjectFileParser.h"

int main(int argc, char *argv[]) {
    std::vector<Commandline::Option> options = {
            Commandline::Option("output", 'o', "file", "write output to FILE"),
            Commandline::Option("depfile", 'M', "file", "write gcc-style dependency file to FILE")
    };

    auto commandline = Commandline(options, "source.o ...", "", "", "");

    auto arguments = commandline.parse(argc, argv);

    try {
        auto parser = ObjectFileParser();
        for (const auto& file: arguments.arguments) {
            auto object_file = parser.parse(file);
        }

        auto output_file = arguments.find_first("output");
        if (!output_file.has_value()) {
            output_file = "a.bin";
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
