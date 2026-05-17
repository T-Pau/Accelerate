/*
Range.h -- 

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

#ifndef ACCELERATE_RANGE_H
#define ACCELERATE_RANGE_H

#include "Int.h"
#include <cstdint>
#include <ostream>

/// Class representing an integer range.
class Range {
public:
    /// @brief Create an empty range.
    Range() = default;

    /**
     * Create a range with a specified start and size.
     * 
     * @param start The starting value of the range.
     * @param size The size of the range.
     */
    Range(uint64_t start, uint64_t size): start(start), size(size) {}

    /**
     * Check if the range is empty.
     * 
     * @return `true` if the range is empty, `false` otherwise.
     */
    [[nodiscard]] bool empty() const {return size == 0;}

    /**
     * Get the last value included in the range.
     * 
     * @return The last value included in the range.
     */
    [[nodiscard]] uint64_t end() const {return empty() ? start : start - 1 + size;} // last byte included in range

    /**
     * Check if the range contains another range.
     * 
     * @param other The range to check.
     * @return `true` if the range contains the other range, `false` otherwise.
     */
    [[nodiscard]] bool contains(const Range& other) const {return other.start >= start && other.end() <= end();}

    /**
     * Get the intersection of the range with another range.
     * 
     * @param other The other range.
     * @return The intersection of the two ranges.
     */
    [[nodiscard]] Range intersect(const Range& other) const;

    /** 
     * Add another range to this range, creating a new range that includes all values included in either range or between them.
     * 
     * @param other The other range to add.
     * @return A new range that includes all values included in either range or between them.
     */
    [[nodiscard]] Range add(const Range& other) const;

    /**
     * Check if two ranges are the same.
     * 
     * @param other The other range to compare with.
     * @return `true` if the ranges are the same, `false` otherwise.
     */
    bool operator==(const Range& other) const {return start == other.start && size == other.size;}

    /**
     * Order two ranges, first by their starting value, then by their size.
     * 
     * @param other The other range to compare with.
     * @return `true` if this range compares less than the other range, `false` otherwise.
     */
    bool operator<(const Range& other) const;

    /**
     * Check if two ranges are different.
     * 
     * @param other The other range to compare with.
     * @return `true` if the ranges are different, `false` otherwise.
     */
    bool operator!=(const Range& other) const {return !(*this == other);}

    /**
     * Extend the range to the left.
     * 
     * @param amount The amount to extend the range by.
     */
    void add_left(uint64_t amount);

    /**
     * Extend the range to the right.
     * 
     * @param amount The amount to extend the range by.
     */
    void add_right(uint64_t amount) {size += amount;}

    /**
     * Adjust the start of the range to a specified alignment without changing its end.
     * 
     * @param alignment The alignment to align the range to.
     * @throws Exception if the resulting range would be empty.
     */
    void align(uint64_t alignment);

    /**
     * Remove a specified amount from the left of the range.
     * 
     * @param amount The amount to remove from the left of the range.
     * @throws Exception if the range is smaller than `amount`.
     */
    void remove_left(uint64_t amount);

    /**
     * Remove a specified amount from the right of the range.
     * 
     * @param amount The amount to remove from the right of the range.
     * @throws Exception if the range is smaller than `amount`.
     */
    void remove_right(uint64_t amount);

    /**
     * Set the starting value of the range, keeping the end fixed.
     * 
     * @param new_start The new starting value of the range.
     * @throws Exception if the `new_start` is greater than the current end.
     */
    void set_start(uint64_t new_start);
    /**
     * Set the ending value of the range, keeping the start fixed.
     * 
     * @param new_end The new ending value of the range.
     * @throws Exception if the `new_end` is less than the current start.
     */
    void set_end(uint64_t new_end);

    /**
     * Serialize the range to a stream.
     * 
     * @param stream The stream to serialize to.
     */
    void serialize(std::ostream& stream) const;

    /// @brief The starting value of the range.
    uint64_t start = 0;

    /// @brief The size of the range. The range includes the values from `start` to `start + size - 1`.
    uint64_t size = 0;
};

/**
 * Output a range to a stream.
 * 
 * @param stream The stream to output to.
 * @param range The range to output.
 * @return The stream that was output to.
 */
std::ostream& operator<<(std::ostream& stream, const Range& range);

#endif //ACCELERATE_RANGE_H
