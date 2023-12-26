/*
Base64.cc --

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

#include "Base64.h"


#include "Exception.h"

void Base64Encoder::encode(const std::string& data) {
    for (const auto datum : data) {
        encode(datum);
    }
}

void Base64Encoder::encode(uint8_t datum) {
    if (ended) {
        throw Exception("can't encode data to finished enocder");
    }

    uint8_t value;
    switch (position) {
        case 0:
            value = datum >> 2;
            remaining_bits = (datum & 0x3) << 4;
            break;

        case 1:
            value = remaining_bits | (datum >> 4);
            remaining_bits = (datum & 0xf) << 2;
            break;

        case 2:
            value = remaining_bits | (datum >> 6);
            remaining_bits = datum & 0x3f;
            break;

        default:
            throw Exception("internal error: invalid position in Base64Encoder");
    }

    add(value);
    if (position == 3) {
        add(remaining_bits);
        position = 0;
    }
}

void Base64Encoder::finish() {
    switch (position) {
        case 0:
            break;

        case 1:
            add(remaining_bits);
            add('=');
            add('=');
            break;

        case 2:
            add(remaining_bits);
            add('=');
            break;

        default:
            throw Exception("internal error: invalid position in Base64Encoder");
    }

    ended = true;
}

char Base64Encoder::character(uint8_t value) {
    if (value <= 26) {
        return static_cast<char>('A' + value);
    }
    else if (value <= 52) {
        return static_cast<char>('a' + value - 26);
    }
    else if (value < 62) {
        return static_cast<char>('0' + value - 52);
    }
    else if (value == 62) {
        return static_cast<char>('+');
    }
    else if (value == 63) {
        return static_cast<char>('/');
    }
    else {
        throw Exception("internal error: invalid value in Base64Encoder");
    }
}

void Base64Encoder::add(uint8_t value) {
    output(character(value));
    position += 1;
}

void Base64StreamEncoder::output(char c) {
    if (line_position >= line_length) {
        stream << std::endl << indent;
        line_position = 0;
    }

    stream << c;
    line_position += 1;
}

void Base64Deocder::decode(const std::string& data) {
    for (const auto character : data) {
        decode(character);
    }
}

void Base64Deocder::decode(char character) {
    if (character == ' ' || character == '\t' || character == '\n' || character == '\r') {
        return;
    }

    if (character == '=') {
        ended = true;
    }

    if (ended) {
        throw Exception("garbage after end marker in base64 data");
    }

    const uint8_t datum = value(character);
    uint8_t value{};

    switch (position) {
        case 0:
            remaining_bits = datum << 2;
            break;

        case 1:
            value = remaining_bits | (datum >> 4);
            remaining_bits = (datum & 0xf) << 4;
            break;

        case 2:
            value = remaining_bits | (datum >> 2);
            remaining_bits = (datum & 0x3) << 6;
            break;

        case 3:
            value = remaining_bits | datum;
            break;

        default:
            throw Exception("internal error: invalid position in Base64Decoder");
    }

    if (position != 0) {
        decoded_string += static_cast<char>(value);
    }
    position = (position + 1) % 4;
}

const std::string& Base64Deocder::end() const {
    if (!ended && position != 0) {
        throw Exception("incomplete base64 data");
    }
    return decoded_string;
}

uint8_t Base64Deocder::value(char character) {
    if (character >= 'A' && character <= 'Z') {
        return character - 'A';
    }
    else if (character >= 'a' && character <= 'z') {
        return character - 'a' + 26;
    }
    else if (character >= '0' && character <= '9') {
        return character - '0' + 52;
    }
    else if (character == '+') {
        return 62;
    }
    else if (character == '/') {
        return 63;
    }
    else {
        throw Exception("invalid character in base64 data");
    }
}
