//
// Created by Dieter Baron on 23.01.23.
//

#include "Path.h"

Symbol Path::find(Symbol name, Symbol base) const {
    auto path = std::filesystem::path(name.str());

    if (std::filesystem::exists(path)) {
        return name;
    }
    else if (path.is_relative()) {
        for (const auto &directory: directories) {
            auto file = directory / path;
            if (std::filesystem::exists(file)) {
                return Symbol(file.lexically_normal());
            }
        }

        if (!base.empty()) {
            auto file = std::filesystem::path(base.str()).parent_path() / path;
            if (std::filesystem::exists(file)) {
                return Symbol(file.lexically_normal());
            }
        }
    }

    return {};
}
