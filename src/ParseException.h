/*
ParseException.h --

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

#ifndef PARSER_EXCEPTION_H
#define PARSER_EXCEPTION_H

#include "Exception.h"

#include <utility>

#include "Location.h"
#include "Token.h"

class ParseException: public Exception {
public:
    class Note {
      public:
        Note(Location location, std::string text) : location(location), text(std::move(text)) {}

        Location location;
        std::string text;
    };

    ParseException(const Token& token, const Exception& exception): ParseException(token.location, exception) {}
    ParseException(Location location, const Exception& exception): Exception(std::string(exception.what())), location(location) {}
    ParseException(Location location, const char *format, ...) PRINTF_LIKE(3, 4);
    ParseException(const Token& token, const char* format, ...) PRINTF_LIKE(3, 4);

    explicit ParseException(Location location, std::string message): Exception(std::move(message)), location(location) { }
    ParseException(const Token& token, std::string message): ParseException(token.location, std::move(message)) {}
    ParseException(Location location, std::string message, std::vector<Note> notes): Exception(std::move(message)), location(location), notes(std::move(notes)) {}

    [[nodiscard]] ParseException appending(Location location, const std::string& text) const;

    Location location;
    std::vector<Note> notes;
};


#endif // PARSER_EXCEPTION_H
