#ifndef CHECKSUM_COMPUTATION_H
#define CHECKSUM_COMPUTATION_H

/*
ChecksumComputation.h --

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
#include <memory>

#include "ChecksumAlgorithm.h"

class ChecksumComputation {
  public:
    ChecksumComputation(std::shared_ptr<ChecksumAlgorithm> algorithm, uint64_t result_position, uint64_t start, uint64_t end, std::unordered_map<Symbol, Value> parameters): algorithm{std::move(algorithm)}, result_position{result_position}, start{start}, end{end}, parameters{std::move(parameters)} {}

    void compute(std::string& data) const;

  private:
    std::shared_ptr<ChecksumAlgorithm> algorithm;
    uint64_t result_position{};
    uint64_t start{};
    uint64_t end{};
    std::unordered_map<Symbol, Value> parameters;
};

#endif // CHECKSUM_COMPUTATION_H
