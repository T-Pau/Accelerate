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

class Memory {
public:
    enum Allocation {
        DATA,
        FREE,
        RESERVED
    };

    class Bank {
    public:
        explicit Bank(Range range);

        std::optional<uint64_t> allocate(const Range& allowed_range, Allocation allocation, uint64_t alignment, uint64_t size);
        void copy(uint64_t start, const std::string& data);

        [[nodiscard]] Range data_range() const;

        std::string data(const Range& range) const;

    private:
        class Block {
        public:
            Block(Allocation allocation, Range range): allocation(allocation), range(range) {}

            Allocation allocation;
            Range range;
        };

        std::string memory;
        Range range;
        std::list<Block> blocks;
    };

    Memory() = default;
    explicit Memory(const std::vector<Range>& bank_ranges);

    Bank& operator[](uint64_t bank) {return banks[bank];}

private:
    std::vector<Bank> banks;
};


#endif //ACCELERATE_MEMORY_H
