/*
HexStringDecoder.h --

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

#ifndef HEX_STRING_DECODER_H
#define HEX_STRING_DECODER_H

#include <cstdint>
#include <optional>
#include <string>

/// @brief A helper class to decode hexadecimal strings.
class HexStringDecoder {
public:
    /**
     * Decode a partial hexadecimal string, ignoring whitespace characters.
     * 
     * @param string The string to decode.
     * @throws Exception If the string contains an invalid character.
     */
    void decode(const std::string& string);

    /**
     * Decode a single hexadecimal character, ignoring whitespace characters.
     * 
     * @param character The character to decode.
     * @throws Exception If the character is not a valid character.
     */
    void decode(char character);

    /**
     * Finish decoding and return the resulting string.
     * 
     * @return The decoded string.
     * @throws Exception If there is a partial hexadecimal left.
     */
    std::string end();

private:
    /**
     * Get the numeric value of a hexadecimal digit.
     * 
     * @param digit The hexadecimal digit.
     * @return The numeric value of the digit.
     */
    static uint8_t value(char digit);

    /// @brief The decoded string so far.
    std::string string;

    /// @brief The partial value of the last decoded hexadecimal digit, if any.
    std::optional<uint8_t> partial;
};



#endif //HEX_STRING_DECODER_H
