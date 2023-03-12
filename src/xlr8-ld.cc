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

//#include <filesystem>
#include <vector>

#include "Command.h"
#include "CPUParser.h"
#include "ObjectFileParser.h"
#include "Exception.h"
#include "MemoryMapParser.h"
#include "Linker.h"

class xlr8_ld: public Command {
public:
    xlr8_ld(): Command(options, "file ...", "xlr8-ld") {}

protected:
    void process() override;
    void create_output() override;
    size_t minimum_arguments() override {return 1;}

private:
    Linker linker;

    static std::vector<Commandline::Option> options;
};

std::vector<Commandline::Option> xlr8_ld::options = {
        Commandline::Option("cpu", "file", "read CPU definition from FILE"),
        Commandline::Option("map", "file", "read memory map from FILE")
};


int main(int argc, char *argv[]) {
    auto command = xlr8_ld();

    return command.run(argc, argv);
}


void xlr8_ld::process() {
    auto cpu_file = arguments.find_first("cpu");
    if (!cpu_file.has_value()) {
        throw Exception("missing option --cpu");
    }
    auto cpu = CPUParser().parse(cpu_file.value());

    auto map_file = arguments.find_first("map");
    if (!map_file.has_value()) {
        throw Exception("missing option --map");
    }
    auto map = MemoryMapParser().parse(map_file.value());

    linker = Linker(std::move(map), std::move(cpu));

    auto parser = ObjectFileParser();

    for (const auto &file_name: arguments.arguments) {
        auto file = parser.parse(file_name);

        auto extension = std::filesystem::path(file_name).extension();
        if (extension == ".o") {
            linker.add_file(file);
        }
        else if (extension == ".lib") {
            linker.add_library(file);
        }
        else {
            throw Exception("unrecognized file type '%s'", extension.c_str());
        }
    }

    linker.link();
}


void xlr8_ld::create_output() {
    linker.output(output_file);
}
