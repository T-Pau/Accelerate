//
// Created by Dieter Baron on 23.01.23.
//

#include "Path.h"

std::optional<std::string> Path::find(const std::string &name, const std::string &base) const {
    auto path = std::filesystem::path(name);

    if (std::filesystem::exists(path)) {
        return path;
    }
    else if (path.is_relative()) {
        for (const auto &directory: directories) {
            auto file = directory / path;
            if (std::filesystem::exists(file)) {
                return file.lexically_normal();
            }
        }

        if (!base.empty()) {
            auto file = std::filesystem::path(base).parent_path() / path;
            if (std::filesystem::exists(file)) {
                return file.lexically_normal();
            }
        }
    }

    return {};
}
