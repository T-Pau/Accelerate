/*
Util.cc -- utility functions

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

#include "Util.h"

#include <cstring>
#include <filesystem>

std::string string_format(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    auto string = string_format_v(format, ap);
    va_end(ap);
    return string;
}


std::string string_format_v(const char *format, va_list ap) {
    auto size = strlen(format) + 50;
    std::string str;
    va_list ap2;
    while (true) {
        str.resize(size);
        va_copy(ap2, ap);
        int n = vsnprintf((char *)str.data(), size, format, ap2);
        va_end(ap2);
        if (n > -1 && static_cast<size_t>(n) < size) {
            str.resize(static_cast<size_t>(n));
            return str;
        }
        if (n > -1) {
            size = static_cast<size_t>(n) + 1;
        }
        else {
            size *= 2;
        }
    }
}


std::string default_output_filename(const std::string& file_name, const std::string& extension) {
    return std::filesystem::path(file_name).stem().filename().string() + "." + extension;
}


std::string join(const std::vector<Symbol>& symbols, const std::string& separator) {
    auto s = std::string();
    auto first = true;

    for (auto& symbol: symbols) {
        if (first) {
            first = false;
        }
        else {
            s += separator;
        }
        s += symbol.str();
    }

    return s;
}
