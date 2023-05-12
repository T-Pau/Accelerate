/*
ErrorBody.cc -- 

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

#include "ErrorBody.h"

std::optional<Body> ErrorBody::evaluated(const Environment &environment, bool top_level, const SizeRange &offset) const {
    if (top_level) {
        throw ParseException(location, message);
    }
    else {
        return {};
    }
}

void ErrorBody::serialize(std::ostream &stream, const std::string &prefix) const {
    stream << prefix << ".error \"" << message << "\" (\"" << location.file.str() << "\"";
    if (location.start_line_number > 0) {
        stream << ", " << location.start_line_number;
        if (location.start_column > 0 || location.end_column > 0) {
            stream << ", " << location.start_column;
            if (location.start_column != location.end_column) {
                stream << ", " << location.end_column;
            }
        }
    }
    stream << ")" << std::endl;
}
