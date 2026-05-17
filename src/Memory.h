/*
Memory.h -- 

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

#ifndef ACCELERATE_MEMORY_H
#define ACCELERATE_MEMORY_H

#include <list>
#include <optional>
#include <string>
#include <vector>

#include "Range.h"

/**
 * Class representing the memory of the target system, including allocated and free blocks and data.
 */
class Memory {
public:
    /// @brief The type of allocation for a block of memory.
    enum Allocation {
        DATA, ///< Block containing data.
        FREE, ///< Free block that can be allocated.
        RESERVED ///< Reserved block.
    };

    /**
     * Class representing a bank of memory.
     */
    class Bank {
    public:
        /**
         * Create a bank of memory.
         * 
         * @param range The range of addresses covered by the bank.
         * @param fill_byte The byte used to fill free and reserved blocks.
         */
        explicit Bank(Range range, uint8_t fill_byte = 0);

        /**
         * Allocate a block of memory within the bank.
         * 
         * @param allowed_range The range of addresses allowed for allocation.
         * @param allocation The type of allocation.
         * @param alignment The alignment requirement.
         * @param size The size of the block to allocate.
         * @return The starting address of the allocated block, or std::nullopt if allocation failed.
         */
        std::optional<uint64_t> allocate(const Range& allowed_range, Allocation allocation, uint64_t alignment, uint64_t size);

        /**
         * Copy data into the memory at a specified address.
         * 
         * @param start The starting address to copy the data to.
         * @param data The data to copy.
         */
        void copy(uint64_t start, const std::string& data);

        /**
         * Get the range of addresses containing data.
         * 
         * @return The range of addresses containing data.
         */
        [[nodiscard]] Range data_range() const;

        /**
         * Get the data within a specified range.
         * 
         * @param requested_range The range of addresses to get data from.
         * @return The data within the specified range.
         */
        [[nodiscard]] std::string data(const Range& requested_range) const;

        /**
         * Debug the blocks within the bank.
         * 
         * @param stream The output stream to write the debug information to.
         */
        void debug_blocks(std::ostream& stream) const;

    private:
        /// @brief Class representing a block of memory within a bank.
        class Block {
        public:
            /**
             * Create a block of memory.
             * 
             * @param allocation The type of allocation for the block.
             * @param range The range of addresses covered by the block.
             */
            Block(Allocation allocation, Range range): allocation(allocation), range(range) {}

            /// @brief The type of allocation for the block.
            Allocation allocation;

            /// @brief The range of addresses covered by the block.
            Range range;
        };

        /**
         * Calculate the offset of an address within the bank.
         * 
         * @param address The address to calculate the offset for.
         * @return The offset of the address within the bank.
         */
        size_t offset(size_t address) const {return address - range.start;}

        /// @brief The memory of the bank.
        std::string memory;

        /// @brief The range of addresses covered by the bank.
        Range range;

        /// @brief The blocks within the bank, sorted by starting address.
        std::list<Block> blocks;
    };

    /// @brief Create an empty memory object.
    Memory() = default;

    /**
     * Create a memory object with specified ranges.
     * 
     * @param bank_ranges The ranges of addresses for each bank.
     * @param fill_byte The byte used to fill free and reserved blocks.
     */
    explicit Memory(const std::vector<Range>& bank_ranges, uint8_t fill_byte = 0);


    /**
     * Get bank by its number.
     * 
     * @param bank The number of the bank to get.
     * @return The bank with the specified number.
     */
    Bank& operator[](uint64_t bank) {return banks[bank];}

        /**
     * Get bank by its number.
     * 
     * @param bank The number of the bank to get.
     * @return The bank with the specified number.
     */
    const Bank& operator[](uint64_t bank) const {return banks[bank];}

private:
    /// @brief The banks of memory.
    std::vector<Bank> banks;
};

#endif //ACCELERATE_MEMORY_H
