//
// Created by Dieter Baron on 23.01.23.
//

#ifndef PATH_H
#define PATH_H

#include <filesystem>
#include <optional>
#include <vector>

#include "Symbol.h"

class Path {
public:
    void append_directory(std::string name) {directories.emplace_back(std::move(name));}
    void prepend_directory(std::string name) {directories.emplace(directories.begin(), std::move(name));}

    [[nodiscard]] bool empty() const {return directories.empty();}
    [[nodiscard]] Symbol find(Symbol name, Symbol base = {}) const;

private:
    std::vector<std::filesystem::path> directories;
};


#endif // PATH_H
