/*
FileParser.cc -- 

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

#include "FileParser.h"

#include "ExpressionParser.h"
#include "ParsedValue.h"
#include "ParseException.h"
#include "FileReader.h"

TokenGroup FileParser::group_directive = TokenGroup({Token::DIRECTIVE,Token::END}, {}, "directive");

FileParser::FileParser() {
    ParsedValue::setup(tokenizer);
    ExpressionParser::setup(tokenizer);
}

bool FileParser::parse_file(const std::string& file_name) {
    tokenizer.push(file_name);

    auto ok = true;

    while (!tokenizer.ended()) {
        try {
            tokenizer.skip(Token::NEWLINE);
            auto token = tokenizer.expect(group_directive, group_directive);
            if (!token) {
                break;
            }
            parse_directive(token);
        }
        catch (ParseException& ex) {
            ok = false;
            FileReader::global.error(ex.location, "%s", ex.what());
            tokenizer.skip_until(group_directive);
        }
    }

    return ok;
}
