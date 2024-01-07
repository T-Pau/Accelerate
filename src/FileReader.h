/*
FileReader.h -- Read File into Array of Lines

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

#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Location.h"
#include "Symbol.h"
#include "printf_like.h"

class FileReader {
public:
    enum DiagnosticsSeverity {
        NOTICE,
        WARNING,
        ERROR
    };

    const std::vector<std::string>& read(Symbol file_name, bool optional = false);
    std::string read_binary(Symbol file_name);

    [[nodiscard]] const std::string& get_line(Symbol file, size_t line_number) const;

    void notice(const Location& location, const char* format, ...) const PRINTF_LIKE(3, 4);
    void notice(const Location& location, const std::string& message) const {output(NOTICE, location, message);}
    void warning(const Location& location, const char* format, ...) const PRINTF_LIKE(3, 4);
    void warning(const Location& location, const std::string& message) const {output(WARNING, location, message);}
    void error(const Location& location, const char* format, ...) PRINTF_LIKE(3, 4);
    void error(const Location& location, const std::string& message) {output(ERROR, location, message); error_flag = true;}

    void output(DiagnosticsSeverity severity, const Location& location, const char* format, ...) const PRINTF_LIKE(4, 5);
    void output(DiagnosticsSeverity severity, const Location& location, const std::string& message) const;

    [[nodiscard]] std::vector<std::string> file_names() const;

    [[nodiscard]] bool had_error() const {return error_flag;}

    static FileReader global;

    bool verbose_error_messages{false};

private:
    static const char *diagnostics_severity_name(DiagnosticsSeverity severity);

    static std::vector<std::string> empty_file;

    std::unordered_set<Symbol> binary_files;
    std::unordered_map<Symbol,std::vector<std::string>> files;
    bool error_flag = false;
    std::ostream& diagnostics_file = std::cerr;

};


#endif // FILEREADER_H
