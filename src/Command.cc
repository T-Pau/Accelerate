/*
Command.cc -- 

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

#include "Command.h"

#include <fstream>
#include <iostream>
#include <utility>

#include "config.h"

#include "FileReader.h"
#include "Exception.h"
#include "Util.h"

std::string Command::header = " -- " PACKAGE " by T'Pau";
std::string Command::footer = "Report bugs to " PACKAGE_BUGREPORT ".";
std::string Command::version = VERSION;

std::vector<Commandline::Option> Command::standard_options = {
        Commandline::Option("depfile", 'M', "file", "write gcc-style dependency file to FILE"),
        Commandline::Option("output", 'o', "file", "write output to FILE")
};


Command::Command(const std::vector<Commandline::Option>& options, std::string arguments, const std::string& name): commandline(standard_options, std::move(arguments), name + header, footer, version) {
    for (const auto& option: options) {
        commandline.add_option(option);
    }
}


int Command::run(int argc, char *const *argv) {
    try {
        program_name = argv[0];
        arguments = commandline.parse(argc, argv);

        if (arguments.arguments.size() < minimum_arguments() || arguments.arguments.size() > maximum_arguments()) {
            commandline.usage(false, stderr);
            throw Exception();
        }

        output_file = arguments.find_first("output");

        process();

        if (FileReader::global.had_error()) {
            throw Exception();
        }

        create_output();

        auto depfile = arguments.find_first("depfile");
        if (depfile.has_value()) {
            if (!output_file.has_value()) {
                throw Exception("can't create depfile with multiple outputs");
            }
            auto stream = std::ofstream(depfile.value());

            stream << output_file.value() << ":";
            for (const auto& file_name: FileReader::global.file_names()) {
                stream << " " << file_name;
            }
            stream << std::endl;
        }
    }
    catch (std::exception& ex) {
        if (strlen(ex.what()) > 0) {
            std::cerr << program_name << ": " << ex.what() << std::endl;
        }
        return 1;
    }

    return 0;
}

void Command::set_output_file(const std::string &file_name, const std::string &extension) {
    output_file = default_output_filename(file_name, extension);
}
