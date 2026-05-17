/*
HexStreamEncoder.h -- 

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

#ifndef HEX_STREAM_ENCODER_H
#define HEX_STREAM_ENCODER_H

#include <cstdint>
#include <ostream>

/// @brief A helper class to encode data a hexadecimal string.
class HexStreamEncoder {
public:
    /**
     * Create a new hex stream encoder.
     * 
     * @param stream The stream to write the encoded data to.
     * @param line_length The maximum length of a line. If 0, there is no maximum length.
     * @param indent The number of spaces to indent each continuation line.
     */
    HexStreamEncoder(std::ostream& stream, size_t line_length = 0, size_t indent = 0): stream{stream}, line_length{line_length}, indent{std::string(indent, ' ')} {}

    /**
     * Encode a character as hexadecimal.
     * 
     * @param character The character to encode.
     */
    void encode(char character);

    /**
     * Encode a string as hexadecimal.
     * 
     * @param string The string to encode.
     */
    void encode(const std::string& string);

private:
    /**
     * Convert a value in the range 0-15 to a hexadecimal digit.
     * 
     * @param value The value to convert.
     * @return The hexadecimal digit corresponding to the value.
     * @throws Exception If the value is not in the range 0-15.
     */
    static char digit(uint8_t value);

    /// @brief The stream to write the encoded data to.
    std::ostream& stream;

    /// @brief The maximum length of a line. If 0, there is no maximum length.
    size_t line_length{};

    /// @brief The current position in the line.
    size_t line_position{};

    /// @brief The indentation string for continuation lines.
    std::string indent;
};

#endif //HEX_STREAM_ENCODER_H
