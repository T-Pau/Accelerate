/*
Base64.h -- 

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

#ifndef BASE64_H
#define BASE64_H

#include <ostream>

class Base64Encoder {
public:
    virtual ~Base64Encoder() = default;

    void encode(const std::string& data);
    void encode(uint8_t datum);

protected:
    void finish();

    virtual void output(char c) = 0;

private:
    bool ended{false};
    int position{0};
    uint8_t remaining_bits{0};

    static char character(uint8_t value);
    void add(uint8_t value);
};

class Base64StringEncoder: public Base64Encoder {
public:
    std::string end() {finish(); return encoded_data;}

protected:
    void output(char c) override {encoded_data += c;}

private:
    std::string encoded_data;
};

class Base64StreamEncoder: public Base64Encoder {
public:
    Base64StreamEncoder(std::ostream& stream, size_t line_length = 0, size_t indent = 0): stream{stream}, line_length{line_length}, indent{std::string(indent, ' ')} {}

    void end() {finish();}

protected:
    void output(char c) override;

private:
    std::ostream& stream;
    size_t line_length;
    std::string indent;

    size_t line_position{0};
};

class Base64Deocder {
public:
    void decode(const std::string& data);
    void decode(char character);

    const std::string& end() const;

private:
    std::string decoded_string;

    bool ended{false};
    int position{0};
    uint8_t remaining_bits{};

    static uint8_t value(char character);
};

#endif //BASE64_H
