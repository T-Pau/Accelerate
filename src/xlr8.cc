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
#include "ParseException.h"

class xlr8: public Command {
public:
    xlr8(): Command(options, "file ...", "xlr8") {}

protected:
    void process() override;
    void create_output() override;
    size_t minimum_arguments() override {return 1;}

private:
    enum Mode {
        COMPILE,
        CREATE_LIBRARY,
        LINK
    };
    class File {
    public:
        File(std::string name, std::shared_ptr<ObjectFile> file): name(std::move(name)), file(std::move(file)) {}
        std::string name;
        std::shared_ptr<ObjectFile> file;
    };

    static std::vector<Commandline::Option> options;

    Mode mode = LINK;

    std::unique_ptr<Linker> linker;
    Path library_path;
    Path include_path;
    Path system_path;

    std::vector<File> files;
};

std::vector<Commandline::Option> xlr8::options = {
        Commandline::Option("compile", 'c', "compile only, don't link"),
        Commandline::Option("create-library", 'a', "create library"),
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
    std::optional<std::string> target_name;
    auto ok = true;

    for (const auto& option: arguments.options) {
        try {
            if (option.name == "compile") {
                mode = COMPILE;
            }
            else if (option.name == "create-library") {
                mode = CREATE_LIBRARY;
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
        }
        catch (Exception& ex) {
            FileReader::global.error({}, "%s", ex.what());
            ok = false;
        }
    }

    if (mode == COMPILE && !library_path.empty()) {
        FileReader::global.warning({}, "compiling only, --library-path not used");
    }

    system_path.append_path(include_path);
    auto system_directory = getenv("XLR8_SYSTEM_DIRECTORY");
    system_path.append_directory(system_directory ? system_directory : SYSTEM_DIRECTORY);

    LibraryGetter::global.path->append_path(library_path);
    LibraryGetter::global.path->append_path(system_path, "lib");
    TargetGetter::global.path->append_path(system_path, "target");
    CPUGetter::global.path->append_path(system_path, "cpu");

    const Target* target = nullptr;
    if (target_name) {
        target = &Target::get(*target_name);
    }
    linker = std::make_unique<Linker>(target);

    for (const auto &file_name: arguments.arguments) {
        try {
            auto extension = std::filesystem::path(file_name).extension();

            if (extension == ".s") {
                files.emplace_back(file_name, Assembler(linker->target).parse(Symbol(file_name)));
            }
            else if (extension == ".o") {
                if (mode == COMPILE) {
                    throw Exception("only compiling, object file not used");
                }
                files.emplace_back(file_name, ObjectFileParser().parse(Symbol(file_name)));
            }
            else if (extension == ".lib") {
                if (mode == COMPILE) {
                    throw Exception("only compiling, library not used");
                }
                linker->add_library(LibraryGetter::global.get(file_name));
            }
            else {
                throw Exception("unrecognized file type '%s'", extension.c_str());
            }
        }
        catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
            ok = false;
        }
        catch (Exception& ex) {
            FileReader::global.error(Location(file_name), "%s", ex.what());
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
                switch (mode) {
                    case COMPILE:
                        break;

                    case CREATE_LIBRARY:
                        set_output_file(files[0].name, "lib");
                        break;

                    case LINK:
                        set_output_file(files[0].name, linker->target->extension);
                        break;
                }
            }
            if (mode == LINK && !output_file.has_value()) {
            }
            break;

        default:
            if (mode != COMPILE) {
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

    if (mode == COMPILE) {
        for (const auto& file: files) {
            for (auto object: file.file->all_objects()) {
                object->evaluate(file.file->local_environment);
            }
        }
    }
    else {
        for (const auto& file: files) {
            linker->add_file(file.file);
        }
        if (mode == LINK) {
            linker->link();
        }
        else {
            // TODO: resolve constants in program
            linker->program->remove_local_constants();
        }
    }
}

void xlr8::create_output() {
    switch (mode) {
        case COMPILE:
            for (const auto& file: files) {
                auto file_name = output_file.has_value() ? output_file.value() : default_output_filename(file.name, "o");
                auto stream = std::ofstream(file_name);
                stream << file.file;
            }
            break;

        case CREATE_LIBRARY: {
            auto stream = std::ofstream(output_file.value());
            stream << *(linker->program);
            break;
        }

        case LINK:
            linker->output(output_file.value());
            break;
    }
}
