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

#include <fstream>
#include <vector>

#include "Command.h"
#include "ObjectFileParser.h"
#include "Exception.h"

class xlr8_ar: public Command {
public:
    xlr8_ar(): Command({}, "object-file ...", "xlr8-ar") {}

protected:
    void process() override;
    void create_output() override;
    size_t minimum_arguments() override {return 1;}

private:
    ObjectFile library;
};


int main(int argc, char *argv[]) {
    auto command = xlr8_ar();

    return command.run(argc, argv);
}


void xlr8_ar::process() {
    if (!output_file.has_value()) {
        throw Exception("missing option --output");
    }

    auto parser = ObjectFileParser();

    for (const auto &file_name: arguments.arguments) {
        auto file = parser.parse(Symbol(file_name));
        library.add_object_file(file);
    }

    library.evaluate(*(library.local_environment));
    library.remove_local_constants();
}


void xlr8_ar::create_output() {
    auto stream = std::ofstream(output_file.value());
    stream << library;
}
