/*
Path.h --

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

#ifndef PATH_H
#define PATH_H

#include <filesystem>
#include <optional>
#include <vector>

#include "Symbol.h"


/// Class representing a search path for files, consisting of a list of directories to search in.
class Path {
public:
    /// @brief An empty path.
    static const Path empty_path;

    /**
     * Append a directory to the search path.
     * 
     * @param directory The directory to append to the search path.
     */
    void append_directory(std::filesystem::path directory) {directories.emplace_back(std::move(directory));}

    /**
     * Prepend a directory to the search path.
     * 
     * @param directory The directory to prepend to the search path.
     */
    void prepend_directory(std::filesystem::path directory) {directories.emplace(directories.begin(), std::move(directory));}

    /**
     * Prepend another path to the search path.
     * 
     * @param path The path to prepend.
     * @param subdirectory An optional subdirectory to add to each directory in the prepended path.
     */
    void prepend_path(const Path& path, const std::string& subdirectory = "");

    /**
     * Append another path to the search path.
     * 
     * @param path The path to append.
     * @param subdirectory An optional subdirectory to add to each directory in the appended path.
     */
    void append_path(const Path& path, const std::string& subdirectory = "");

    /**
     * Check if the search path is empty.
     * 
     * @return `true` if the search path is empty, `false` otherwise.
     */
    [[nodiscard]] bool empty() const {return directories.empty();}

    /**
     * Find a file in the search path.
     * 
     * @param name The name of the file to find.
     * @param base An optional base directory to resolve relative paths against.
     * @return The name to the found file, or the empty symbol if the file was not found.
     */
    [[nodiscard]] Symbol find(Symbol name, Symbol base = {}) const;

private:
    /// @brief The directories in the search path, in order of precedence.
    std::vector<std::filesystem::path> directories;
};


#endif // PATH_H
