//
// Created by Dieter Baron on 23.01.23.
//

#ifndef PATH_H
#define PATH_H

#include <filesystem>
#include <optional>
#include <vector>

class Path {
public:
    void prepend_directory(std::string name) {directories.emplace(directories.begin(), std::move(name));}
    void append_directory(std::string name) {directories.emplace_back(std::move(name));}

    [[nodiscard]] std::optional<std::string> find(const std::string& name, const std::string& base = "") const;

private:
    std::vector<std::filesystem::path> directories;
};


#endif // PATH_H
