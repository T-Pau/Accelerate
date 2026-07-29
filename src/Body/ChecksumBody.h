#ifdef IN_XLR8_CHECKSUM_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_CHECKSUM_BODY_H
#ifndef HAD_XLR8_CHECKSUM_BODY_H
#define HAD_XLR8_CHECKSUM_BODY_H

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

#include <utility>

#include <tpau-cpp-kernal/Exception.h>
#include <tpau-cpp-kernal/Symbol.h>

#include "BodyElement.h"
#include "ChecksumAlgorithm.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a body element that computes a checksum over a range of memory: `.checksum algorithm start end [parameters]`.
 */
class ChecksumBody : public BodyElement {
  public:
    ChecksumBody(std::shared_ptr<ChecksumAlgorithm> algorithm_, Expression start, Expression end, std::unordered_map<Symbol, Expression> parameters) : algorithm{std::move(algorithm_)}, start{std::move(start)}, end{std::move(end)}, parameters{std::move(parameters)} { size_range_ = SizeRange{algorithm->result_size(), algorithm->result_size()}; }

    static Body parse(Tokenizer& tokenizer);

    Body clone(const CloneContext& context) const override;

    bool empty() const override { return false; }

    void encode(std::string& bytes, const Memory* memory) override;
    void traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) override;
    std::optional<Body> evaluate_process(const EvaluationContext& context) override;
    void serialize(std::ostream& stream, const std::string& prefix) const override;

  private:
    std::shared_ptr<ChecksumAlgorithm> algorithm;
    Expression start;
    Expression end;
    std::unordered_map<Symbol, Expression> parameters;
};


#endif // HAD_XLR8_CHECKSUM_BODY_H
#undef IN_XLR8_CHECKSUM_BODY_H
