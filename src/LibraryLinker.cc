/*
Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

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

#include "LibraryLinker.h"

#include <fstream>

#include <tpau-cpp-kernal/Exception.h>
#include <tpau-cpp-kernal/Util.h>

#include "EvaluationOrder.h"

using namespace tpau::cpp_kernal;

const unsigned int LibraryLinker::format_version_major = 1;
const unsigned int LibraryLinker::format_version_minor = 0;

const std::string& LibraryLinker::library_extension = "lib";

void LibraryLinker::output(const std::filesystem::path& file_name) {
    auto stream = std::ofstream(file_name);

    if (!stream) {
        throw Exception("cannot open {} for writing", file_name.string());
    }

    stream << ".format_version " << format_version_major << "." << format_version_minor << std::endl;

    if (target && !target->name.empty()) {
        stream << ".target \"" << target->name.str() << "\"" << std::endl;
    }

    print_imported_modules(stream, Visibility::PRIVATE);
    print_imported_modules(stream, Visibility::PUBLIC);

    // TODO: output pinned and used objects.

    output_entities<Constant>(stream);
    output_entities<Function>(stream);
    output_entities<Macro>(stream);
    output_entities<Object>(stream);
}

void LibraryLinker::print_imported_modules(std::ostream& stream, Visibility visibility) {
    auto imported_modules = module().imported_modules(visibility);

    auto names = std::vector<Symbol>();
    for (const auto& imported_module : imported_modules) {
        names.emplace_back(imported_module->name());
    }
    std::sort(names.begin(), names.end());
    for (const auto& name : names) {
        stream << ".import " << (visibility == Visibility::PRIVATE ? "" : "public ") << "\"" << name.str() << "\"" << std::endl;
    }
}
