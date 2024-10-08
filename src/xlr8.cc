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
#include "CPUGetter.h"
#include "CPUParser.h"
#include "Command.h"
#include "Exception.h"
#include "LibraryGetter.h"
#include "LibraryLinker.h"
#include "ParseException.h"
#include "ProgramLinker.h"
#include "TargetGetter.h"
#include "config.h"

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

    std::unique_ptr<Linker> linker;
    Path library_path;
    Path include_path;
    Path system_path;
    std::unordered_set<Symbol> defines;

    std::vector<File> files;
};

std::vector<Commandline::Option> xlr8::options = {
    Commandline::Option("create-library", 'a', "create library"),
    Commandline::Option("define", 'D', "name", "define NAME for use in conditional compilation"),
    Commandline::Option("include-directory", 'I', "directory", "search for sources in DIRECTORY"),
    Commandline::Option("library-directory", 'L', "directory", "search for libraries in DIRECTORY"),
    Commandline::Option("symbol-map", "file", "write symbol map to FILE"),
    Commandline::Option("system-directory", "directory", "search for system files in DIRECTORY"),
    Commandline::Option("target", "file", "read target definition from FILE"),
    Commandline::Option("undefine", "name", "remove definition of NAME for use in conditional compilation"),
    Commandline::Option("verbose-errors", "include body in error messages")
};


int main(int argc, char *argv[]) {
    auto command = xlr8();

    return command.run(argc, argv);
}


void xlr8::process() {
    std::optional<std::string> target_name;
    auto create_program = true;
    auto ok = true;

    for (const auto& option: arguments.options) {
        try {
            if (option.name == "create-library") {
                create_program = false;
            }
            else if (option.name == "define") {
                defines.insert(Symbol(option.argument));
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
                target_name = option.argument;
            }
            else if (option.name == "undefine") {
                defines.erase(Symbol(option.argument));
            }
            else if (option.name == "verbose-errors") {
                FileReader::global.verbose_error_messages = true;
            }
        }
        catch (Exception& ex) {
            FileReader::global.error({}, "%s", ex.what());
            ok = false;
        }
    }

    system_path.append_path(include_path);
    const auto system_directory = getenv("XLR8_SYSTEM_DIRECTORY");
    system_path.append_directory(system_directory ? system_directory : SYSTEM_DIRECTORY);

    if (getenv("XLR8_VERBOSE_ERRORS")) {
        FileReader::global.verbose_error_messages = true;
    }

    LibraryGetter::global.path->append_path(library_path);
    LibraryGetter::global.path->append_path(system_path, "lib");
    TargetGetter::global.path->append_path(system_path, "target");
    CPUGetter::global.path->append_path(system_path, "cpu");

    if (create_program) {
        linker = std::make_unique<ProgramLinker>();
    }
    else {
        linker = std::make_unique<LibraryLinker>();
    }

    if (target_name) {
        linker->set_target(&Target::get(*target_name));
    }

    for (const auto &file_name: arguments.arguments) {
        try {
            auto extension = std::filesystem::path(file_name).extension();

            if (extension == ".s") {
                files.emplace_back(file_name, Assembler(linker->target, include_path, defines).parse_object_file(Symbol(file_name)));
            }
            else if (extension == ".lib") {
                Target::clear_current_target();
                linker->add_library(LibraryGetter::global.get(file_name));
                Target::clear_current_target();
            }
            else {
                throw Exception("unrecognized file type '%s'", extension.c_str());
            }
        }
        catch (Exception& ex) {
            FileReader::global.error(ex, Location(file_name));
            ok = false;
        }
    }

    if (!ok) {
        throw Exception();
    }

    for (const auto& file: files) {
        linker->set_target(file.file->target);
    }

    if (!linker->target) {
        throw Exception("no target specified");
    }

    switch (files.size()) {
        case 0:
            throw Exception("no sources given");

        case 1:
            if (!output_file) {
                if (create_program) {
                    set_output_file(files[0].name, linker->target->extension);
                }
                else {
                    set_output_file(files[0].name, "lib");
                }
            }
            break;

        default:
            if (!output_file.has_value()) {
                throw Exception("option --output required with multiple source files");
            }
            break;
    }

    for (const auto& file: files) {
        try {
            linker->add_file(file.file);
        }
        catch (Exception& ex) {
            FileReader::global.error(ex);
            ok = false;
        }
    }
    if (!ok) {
        throw Exception();
    }

    linker->link();
}

void xlr8::create_output() {
    linker->output(output_file.value());

    if (auto program_linker = linker->as_program_linker()) {
        if (auto map_file = arguments.find_last("symbol-map")) {
            program_linker->output_symbol_map(*map_file);
        }
    }
}
