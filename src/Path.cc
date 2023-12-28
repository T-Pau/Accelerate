/*
Path.cc --

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

#include "Path.h"

const Path Path::empty_path;

Symbol Path::find(Symbol name, Symbol base) const {
    auto path = std::filesystem::path(name.str());

    if (std::filesystem::exists(path)) {
        return name;
    }
    else if (path.is_relative()) {
        for (const auto &directory: directories) {
            auto file = directory / path;
            if (std::filesystem::exists(file)) {
                return Symbol(file.lexically_normal().string());
            }
        }

        if (!base.empty()) {
            auto file = std::filesystem::path(base.str()).parent_path() / path;
            if (std::filesystem::exists(file)) {
                return Symbol(file.lexically_normal().string());
            }
        }
    }

    return {};
}


void Path::append_path(const Path &path, const std::string &subdirectory) {
    for (const auto& directory: path.directories) {
        if (!subdirectory.empty()) {
            append_directory(directory / subdirectory);
        }
        else {
            append_directory(directory);
        }
    }
}


void Path::prepend_path(const Path &path, const std::string &subdirectory) {
    for (const auto& directory: path.directories) {
        if (!subdirectory.empty()) {
            prepend_directory(directory / subdirectory);
        }
        else {
            prepend_directory(directory);
        }
    }
}
