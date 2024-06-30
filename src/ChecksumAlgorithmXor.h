#ifndef CHECKSUM_ALGORITHM_XOR_H
#define CHECKSUM_ALGORITHM_XOR_H

/*
ChecksumAlgorithmXor.h --

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

#include "ChecksumAlgorithm.h"

class ChecksumAlgorithmXor: public ChecksumAlgorithm {
  public:
    ChecksumAlgorithmXor(Symbol name): ChecksumAlgorithm(name) {}
    static std::shared_ptr<ChecksumAlgorithm> create(Symbol algorithm_name){return std::make_shared<ChecksumAlgorithmXor>(algorithm_name);}

    uint64_t result_size() override {return 1;}
    std::string compute(std::string::const_iterator begin, std::string::const_iterator end, const std::unordered_map<Symbol, Value> &parameters) override;
};

#endif // CHECKSUM_ALGORITHM_XOR_H
