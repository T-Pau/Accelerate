#ifdef IN_XLR8_REPEAT_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_REPEAT_BODY_H
#ifndef HAD_XLR8_REPEAT_BODY_H
#define HAD_XLR8_REPEAT_BODY_H

/*
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

#include <tpau-cpp-kernal/Exception.h>

#include "Body/Body.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Abstract base class for a body that repeats a block of code a specified number of times.
 */
class RepeatBody : public BodyElement {
  public:
    class RepeatRange {
      public:
        RepeatRange(Expression start, Expression end);

        RepeatRange clone(const CloneContext& context) const { return RepeatRange(start.clone(context), end.clone(context)); }

        [[nodiscard]] std::optional<uint64_t> count() const;

        [[nodiscard]] SizeRange count_range() const;

        Location location;
        Expression start;
        Expression end;
    };

    RepeatBody(RepeatRange range, Body body) : range{std::move(range)}, body{std::move(body)} {}

  protected:
    void traverse(std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) override;

    RepeatRange range;
    Body body;
};

std::ostream& operator<<(std::ostream& stream, const RepeatBody::RepeatRange& range);

#endif // HAD_XLR8_REPEAT_BODY_H
#undef IN_XLR8_REPEAT_BODY_H
