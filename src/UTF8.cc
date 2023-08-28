/*
UTF8.cc --

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

#include "UTF8.h"
#include "Exception.h"

const int UTF8::utf8_continuation_bytes[] = {
    /* 0xc0 */ 1, 1, 1, 1,
    /* 0xe0 */ 2, 2,
    /* 0xf0 */ 3
};

const UTF8::FirstByte UTF8::first_bytes[] = {
    {0x00, 0x7f, 0x7f},
    {0xc0, 0x1f, 0x7ff},
    {0xe0, 0x0f, 0xffff},
    {0xf0, 0x03, 0x10ffff}
};


std::u32string UTF8::decode(const std::string& utf8_string) {
    std::u32string ucs32_string;

    int continuation_bytes{};
    char32_t c32{};

    for (auto c: utf8_string) {
        auto byte = static_cast<uint8_t>(c);
        if (continuation_bytes == 0) {
            if (byte < 0x80) {
                ucs32_string += static_cast<char32_t>(c);
            }
            else if (byte < 0xc0 || byte >= 0xf8) {
                throw Exception("invalid UTF-8 string");
            }
            else {
                continuation_bytes = utf8_continuation_bytes[(byte & 0x3f) >> 3];
                c32 = byte & first_bytes[continuation_bytes].mask;
            }
        }
        else {
            if (byte < 0x80 || byte >= 0xc0) {
                throw Exception("invalid UTF-8 string");
            }
            c32 = (c32 << 6) | (byte & 0x3f);
            continuation_bytes -= 1;
            if (continuation_bytes == 0) {
                ucs32_string += c32;
            }
        }
    }

    if (continuation_bytes > 0) {
        throw Exception("invalid UTF-8 string");
    }

    return ucs32_string;
}

std::string UTF8::encode(const std::u32string& ucs32_string) {
    std::string utf8_string;

    for (auto c: ucs32_string) {
        encode(c, utf8_string);
    }

    return utf8_string;
}

std::string UTF8::encode(char32_t codepoint) {
    std::string utf8_string;

    encode(codepoint, utf8_string);

    return utf8_string;
}

void UTF8::encode(char32_t codepoint, std::string& string) {
    if (codepoint < 0x80) {
        string += static_cast<char>(codepoint);
        return;
    }

    for (auto continuation_characters = 1; continuation_characters < 3; continuation_characters+= 1) {
        auto const& first_byte = first_bytes[continuation_characters];
        if (codepoint < first_byte.max_char) {
            string += static_cast<char>((codepoint >> (continuation_characters * 6)) | first_byte.prefix);
            for (auto i = 0; i < continuation_characters; i++) {
                string += static_cast<char>(((codepoint >> ((continuation_characters - i - 1) * 6)) & 0x3f) | 0x80);
            }
            return;
        }
    }

    throw Exception("invalid unicode character");
}
