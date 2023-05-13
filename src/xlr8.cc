/*
xlr8.cc -- 

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

#include "Assembler.h"
#include "CPUParser.h"
#include "Command.h"
#include "Exception.h"
#include "LibraryGetter.h"
#include "Linker.h"
#include "ObjectFileParser.h"
#include "TargetGetter.h"
#include "Util.h"
#include "config.h"
#include "CPUGetter.h"

class xlr8: public Command {
public:
    xlr8(): Command(options, "file ...", "xlr8") {}

protected:
    void process() override;
    void create_output() override;
    size_t minimum_arguments() override {return 1;}

private:
    class File {
    public:
        File(std::string name, std::shared_ptr<ObjectFile> file): name(std::move(name)), file(std::move(file)) {}
        std::string name;
        std::shared_ptr<ObjectFile> file;
    };

    static std::vector<Commandline::Option> options;

    bool do_link = true;

    std::unique_ptr<Linker> linker;
    Path library_path;
    Path include_path;
    Path system_path;

    std::vector<File> files;
};

std::vector<Commandline::Option> xlr8::options = {
        Commandline::Option("compile", 'c', "compile only, don't link"),
        Commandline::Option("include-directory", 'I', "directory", "search for sources in DIRECTORY"),
        Commandline::Option("library-directory", 'L', "directory", "search for libraries in DIRECTORY"),
        Commandline::Option("system-directory", "directory", "search for system files in DIRECTORY"),
        Commandline::Option("target", "file", "read target definition from FILE"),
};


int main(int argc, char *argv[]) {
    auto command = xlr8();

    return command.run(argc, argv);
}


void xlr8::process() {
    std::optional<std::string> target;
    auto ok = true;

    for (const auto& option: arguments.options) {
        try {
            if (option.name == "compile") {
                do_link = false;
            }
            else if (option.name == "include-directory") {
                include_path.append_directory(option.argument);
            }
            else if (option.name == "library-directory") {
                library_path.append_directory(option.argument);
            }
            else if (option.name == "system-directory") {
                system_path.append_directory(option.argument);
            }
            else if (option.name == "target") {
                target = option.argument;
            }
        }
        catch (Exception& ex) {
            FileReader::global.error({}, "%s", ex.what());
            ok = false;
        }
    }

    if (!do_link && !library_path.empty()) {
        FileReader::global.warning({}, "not linking, --library-path not used");
    }

    system_path.append_path(include_path);
    auto system_directory = getenv("XLR8_SYSTEM_DIRECTORY");
    system_path.append_directory(system_directory ? system_directory : SYSTEM_DIRECTORY);

    LibraryGetter::global.path->append_path(library_path);
    TargetGetter::global.path->append_path(system_path, "target");
    CPUGetter::global.path->append_path(system_path, "cpu");

    if (!target.has_value()) {
        FileReader::global.error({}, "missing --target option");
        ok = false;
    }

    linker = std::make_unique<Linker>(Target::get(target.value()));

    for (const auto &file_name: arguments.arguments) {
        try {
            auto extension = std::filesystem::path(file_name).extension();

            if (extension == ".s") {
                files.emplace_back(file_name, Assembler(linker->target).parse(Symbol(file_name)));
            }
            else if (extension == ".o") {
                if (!do_link) {
                    throw Exception("not linking, object file not used");
                }
                files.emplace_back(file_name, ObjectFileParser().parse(Symbol(file_name)));
            }
            else if (extension == ".lib") {
                if (!do_link) {
                    throw Exception("not linking, library not used");
                }
                linker->add_library(LibraryGetter::global.get(file_name));
            }
            else {
                throw Exception("unrecognized file type '%s'", extension.c_str());
            }
        }
        catch (Exception& ex) {
            FileReader::global.error(Location(file_name), "%s", ex.what());
        }
    }

    if (!ok) {
        throw Exception();
    }

    switch (files.size()) {
        case 0:
            throw Exception("no sources given");

        case 1:
            if (do_link && !output_file.has_value()) {
                set_output_file(files[0].name, linker->target.extension);
            }
            break;

        default:
            if (do_link) {
                if (!output_file.has_value()) {
                    throw Exception("option --output required with multiple source files");
                }
            }
            else {
                if (output_file.has_value()) {
                    throw Exception("option --output not allowed when assembling multiple source files");
                }
            }
            break;
    }

    if (do_link) {
        for (const auto& file: files) {
            linker->add_file(file.file);
        }
        linker->link();
    }
}

void xlr8::create_output() {
    if (do_link) {
        linker->output(output_file.value());
    }
    else {
        for (const auto& file: files) {
            auto file_name = output_file.has_value() ? output_file.value() : default_output_filename(file.name, "o");
            auto stream = std::ofstream(file_name);
            stream << file.file;
        }
    }
}
