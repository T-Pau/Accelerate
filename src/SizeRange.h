#ifdef IN_XLR8_SIZE_RANGE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_SIZE_RANGE_H
#ifndef HAD_XLR8_SIZE_RANGE_H
#define HAD_XLR8_SIZE_RANGE_H

/*
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

#include <cstdint>
#include <optional>
#include <ostream>

#include <tpau-cpp-kernal/Value.h>

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a possibly unbounded range of sizes.
 */
class SizeRange {
public:
    /**
     * @brief Construct a SizeRange representing unknown size.
     */
    SizeRange() = default;

    /**
     * @brief Construct a SizeRange representing a known size.
     *
     * @param size The size of the range.
     */
    explicit SizeRange(uint64_t size): minimum(size), maximum(size) {}
    /**
     * @brief Construct a SizeRange with a minimum and optional maximum.
     *
     * @param minimum The minimum size.
     * @param maximum The maximum size. If not set, the range is unbounded.
     */
    SizeRange(uint64_t minimum, std::optional<uint64_t> maximum): minimum(minimum), maximum(maximum) {}

    /**
     * @brief Get the maximum value of the size range.
     *
     * @return The maximum value, if any.
     */
    [[nodiscard]] std::optional<Value> maximum_value() const {return maximum ? Value(*maximum) : std::optional<Value>();}

    /**
     * @brief Get the minimum value of the size range.
     *
     * @return The minimum value.
     */
    [[nodiscard]] Value minimum_value() const {return Value(minimum);}

    /**
     * @brief If the size range has a known size, return it as a Value.
     *
     * @return The size of the range as a Value, {} if it is unknown.
     */
    [[nodiscard]] std::optional<Value> value() const;

    /**
     * @brief Get the size of the range, if it is known.
     *
     * @return The size of the range, {} if it is unknown.
     */
    [[nodiscard]] std::optional<uint64_t> size() const;

    /**
     * @brief Check if the size of the range is known.
     *
     * @return `true` if the size is known, `false` otherwise.
     */
    [[nodiscard]] bool has_size() const {return maximum.has_value() && minimum == *maximum;}

    /**
     * @brief Get the maximum of this size range and another.
     *
     * It takes the maximum of the minimums and maximums of the two ranges.
     *
     * @param other The other size range to compare with.
     * @return A new SizeRange representing the maximum of the two ranges.
     */
    SizeRange max(const SizeRange& other);

    /**
     * @brief Compare two size ranges for equality.
     *
     * @param other The size range to compare with.
     * @return `true` if the size ranges are equal, `false` otherwise.
     */
    bool operator==(const SizeRange&other) const {return minimum == other.minimum && maximum == other.maximum;}

    /**
     * @brief Compare two size ranges for inequality.
     *
     * @param other The size range to compare with.
     * @return `true` if the size ranges are not equal, `false` otherwise.
     */
    bool operator!=(const SizeRange&other) const {return !(*this == other);}

    /**
     * @brief Subtract another size range from this one.
     *
     * @param other The size range to subtract.
     * @return The resulting size range.
     */
    SizeRange operator-(const SizeRange& other) const;

    /**
     * @brief Add another size range to this one.
     *
     * @param other The size range to add.
     * @return The resulting size range.
     */
    SizeRange operator+(const SizeRange& other) const;

    /**
     * @brief Add another size range to this one and assign the result to this range.
     *
     * @param other The size range to add.
     * @return The resulting size range.
     */
    SizeRange operator+=(const SizeRange& other) {*this = *this + other; return *this;}

    /// The minimum size of the range.
    uint64_t minimum{0};

    /// The maximum size of the range. If not set, the range is unbounded.
    std::optional<uint64_t> maximum;
};

/**
 * @brief Output a SizeRange to a stream.
 *
 * @param stream The stream to output to.
 * @param size_range The SizeRange to output.
 * @return The stream after outputting the SizeRange.
 */
std::ostream& operator<<(std::ostream& stream, const SizeRange& size_range);

#endif // HAD_XLR8_SIZE_RANGE_H
#undef IN_XLR8_SIZE_RANGE_H
