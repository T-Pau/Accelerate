#ifndef CHECKSUM_BODY_H
#define CHECKSUM_BODY_H

/*
ChecksumBody.h --

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

#include <utility>

#include "BodyElement.h"
#include "ChecksumAlgorithm.h"
#include "Exception.h"
#include "Symbol.h"

class ChecksumBody: public BodyElement {
  public:
    ChecksumBody(std::shared_ptr<ChecksumAlgorithm> algorithm_, Expression start, Expression end, std::unordered_map<Symbol, Expression> parameters): algorithm{std::move(algorithm_)}, start{std::move(start)}, end{std::move(end)}, parameters{std::move(parameters)} {size_range_ = SizeRange{algorithm->result_size(), algorithm->result_size()};}

    static Body parse(Tokenizer& tokenizer);

    std::shared_ptr<BodyElement> clone() const override {throw Exception("can't clone .checksum");}
    bool empty() const override {return false;}
    void encode(std::string &bytes, const Memory *memory) const override;
    std::optional<Body> evaluated(const EvaluationContext &context) const override;
    void serialize(std::ostream &stream, const std::string &prefix) const override;

  private:
    std::shared_ptr<ChecksumAlgorithm> algorithm;
    Expression start;
    Expression end;
    std::unordered_map<Symbol, Expression> parameters;
};


#endif // CHECKSUM_BODY_H
