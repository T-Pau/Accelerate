/*
FileReader.cc -- Read File into Array of Lines

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

#include "FileReader.h"

#include <algorithm>
#include <cstring>

#include "ParseException.h"
#include "Util.h"

#include <fstream>

FileReader FileReader::global;

std::vector<std::string> FileReader::empty_file;

const std::vector<std::string>& FileReader::read(Symbol file_name, bool optional) {
    const auto it = files.find(file_name);
    if (it != files.end()) {
        return it->second;
    }

    std::ifstream input_file(file_name.str());
    if (input_file) {
        std::vector<std::string> lines;
        std::string s;
        while (getline(input_file, s)) {
            lines.push_back(s);
        }

        files[file_name] = std::move(lines);
        return files[file_name];
    }
    else if (errno == EEXIST && optional) {
        return empty_file;
    }
    else {
        error_flag = true;
        throw Exception("can't open '%s': %s", file_name.c_str(), strerror(errno));
    }
}

std::string FileReader::read_binary(Symbol file_name) {
    if (std::ifstream input_file{file_name.str()}) {
        return std::string{std::istreambuf_iterator<char>{input_file}, {}};
    }
    else {
        error_flag = true;
        throw Exception("can't open '%s': %s", file_name.c_str(), strerror(errno));
    }
}

const std::string &FileReader::get_line(Symbol file, size_t line_number) const {
    const auto it = files.find(file);

    if (it == files.end()) {
        throw Exception("unknown file '%s'",file.c_str());
    }

    if (line_number == 0 || line_number > it->second.size()) {
        throw Exception("line integer %zu out of range in '%s'", line_number, file.c_str());
    }

    return it->second[line_number - 1];
}


const char* FileReader::diagnostics_severity_name(DiagnosticsSeverity severity) {
    switch (severity) {
        case NOTICE:
            return "notice";

        case WARNING:
            return "warning";

        case ERROR:
            return "error";
    }

    throw Exception("invalid severity");
}


void FileReader::notice(const Location &location, const char *format, ...) const {
    va_list ap;
    va_start(ap, format);
    const auto message = string_format_v(format, ap);
    va_end(ap);
    notice(location, message);
}


void FileReader::warning(const Location &location, const char *format, ...) const {
    va_list ap;
    va_start(ap, format);
    const auto message = string_format_v(format, ap);
    va_end(ap);
    warning(location, message);
}


void FileReader::error(const Location &location, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    const auto message = string_format_v(format, ap);
    va_end(ap);
    error(location, message);
}


void FileReader::output(FileReader::DiagnosticsSeverity severity, const Location &location, const char *format, ...) const {
    va_list ap;
    va_start(ap, format);
    const auto message = string_format_v(format, ap);
    va_end(ap);
    output(severity, location, message);
}

void FileReader::output(FileReader::DiagnosticsSeverity severity, const Location &location, const std::string &message) const {
    const auto location_string = location.to_string();
    if (!location_string.empty()) {
        diagnostics_file << location_string << ": ";
    }
    diagnostics_file << diagnostics_severity_name(severity) << ": " << message << std::endl;

    try {
        const auto line = get_line(location.file, location.start_line_number);
        diagnostics_file << line << std::endl;
        auto width = location.end_column - location.start_column;
        if (width < 1) {
            width = 1;
        }
        for (auto i = 0; i < location.start_column; i++) {
            diagnostics_file << ' ';
        }
        for (auto i = 0; i < width; i++) {
            diagnostics_file << '^';
        }
        diagnostics_file << std::endl;
    }
    catch (...) {}
}

std::vector<std::string> FileReader::file_names() const {
    auto file_names = std::vector<std::string>();

    for (const auto& pair: files) {
        file_names.emplace_back(pair.first.str());
    }
    for (const auto& name: binary_files) {
        file_names.emplace_back(name.str());
    }

    std::sort(file_names.begin(), file_names.end());

    return file_names;
}

void FileReader::error(const Exception& exception, const Location& default_location) {
    if (exception.empty()) {
        error_flag = true;
        return;
    }
    if (auto parsed_exception = dynamic_cast<const ParseException*>(&exception)) {

        error(parsed_exception->location.empty() ? default_location : parsed_exception->location, "%s", exception.what());
        for (const auto& note: parsed_exception->notes) {
            notice(note.location, note.text);
        }
    }
    else {
        error({}, "%s", exception.what());
    }
}
