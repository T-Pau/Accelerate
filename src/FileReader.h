/*
FileReader.h -- Read File into Array of Lines

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

#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Location.h"
#include "printf_like.h"
#include "SymbolTable.h"

class FileReader {
public:
    enum DiagnosticsSeverity {
        NOTICE,
        WARNING,
        ERROR
    };

    const std::vector<std::string>& read(const std::string& file_name, bool optional = false);

    [[nodiscard]] const std::string& get_line(symbol_t file, size_t line_number) const;

    void notice(const Location& location, const char* format, ...) PRINTF_LIKE(3, 4);
    void notice(const Location& location, const std::string& message) {output(NOTICE, location, message);}
    void warning(const Location& location, const char* format, ...) PRINTF_LIKE(3, 4);
    void warning(const Location& location, const std::string& message) {output(WARNING, location, message);}
    void error(const Location& location, const char* format, ...) PRINTF_LIKE(3, 4);
    void error(const Location& location, const std::string& message) {output(ERROR, location, message);}

    void output(DiagnosticsSeverity severity, const Location& location, const char* format, ...) PRINTF_LIKE(4, 5);
    void output(DiagnosticsSeverity severity, const Location& location, const std::string& message);

    [[nodiscard]] bool had_error() const {return error_flag;}

    static FileReader global;

private:
    static const char *diagnostics_severity_name(DiagnosticsSeverity severity);

    static std::vector<std::string> empty_file;

    std::map<symbol_t,std::vector<std::string>> files;
    bool error_flag = false;
    std::ostream& diagnostics_file = std::cerr;

};


#endif // FILEREADER_H
