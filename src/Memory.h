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
#include <string>
#include <vector>

class Memory {
public:
    enum Allocation {
        DATA,
        FREE,
        RESERVED
    };

    class Range {
    public:
        Range(size_t start, size_t end): start(start), end(end) {}

        [[nodiscard]] size_t size() const {return end - start;}
        [[nodiscard]] Range intersect(const Range& other) const;

        size_t start;
        size_t end;
    };

    class Bank {
    public:
        explicit Bank(Range range);

        size_t allocate(const Range& allowed_range, size_t alignment, const std::string& data);
        size_t reserve(const Range& allowed_range, size_t alignment, size_t size) {return allocate(allowed_range, RESERVED, alignment, size);}

        [[nodiscard]] size_t data_start() const;
        [[nodiscard]] size_t data_end() const;

    private:
        class Block {
        public:
            Block(Allocation allocation, Range range): allocation(allocation), range(range) {}

            Allocation allocation;
            Range range;
        };

        size_t allocate(const Range& allowed_range, Allocation allocation, size_t alignment, size_t size);

        std::string memory;
        Range range;
        std::list<Block> blocks;
    };

private:
    std::vector<Bank> banks;
};


#endif //ACCELERATE_MEMORY_H
