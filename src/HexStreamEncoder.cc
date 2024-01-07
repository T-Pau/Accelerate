/*
HexStreamEncoder.cc -- 

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

#include "HexStreamEncoder.h"


#include "Exception.h"

void HexStreamEncoder::encode(const std::string& string) {
    for (const auto character : string) {
        encode(character);
    }
}

void HexStreamEncoder::encode(char character) {
    if (line_length > 0 && line_position + 2 > line_length) {
        stream << std::endl << indent;
        line_position = 0;
    }

    stream << digit(static_cast<uint8_t>(character) >> 4) << digit(static_cast<uint8_t>(character) & 0xf);
    line_position += 2;
}

char HexStreamEncoder::digit(uint8_t value) {
    if (value < 10) {
        return static_cast<char>(value + '0');
    }
    else if (value < 16) {
        return static_cast<char>(value - 10 + 'a');
    }
    else {
        throw Exception("internal error: invalid nibble in hex encoder");
    }
}

