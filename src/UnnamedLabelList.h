#ifndef HAD_XLR8_UNNAMED_LABEL_LIST_H
#define HAD_XLR8_UNNAMED_LABEL_LIST_H

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

#include <vector>

#include <tpau-cpp-kernal/Location.h>

#include "Expression/Expression.h"

using namespace tpau::cpp_kernal;

class UnnamedLabelList {
  public:
    void add_label(const Location& location, Expression expression);
    std::optional<Expression> get_next_label(const Location& location) const;
    std::optional<Expression> get_previous_label(const Location& location) const;
    size_t size() const {return labels.size();}

  private:
    class Label {
      public:
        Label(const Location& location, Expression expression): location(location), expression(std::move(expression)) {}

        Location location;
        Expression expression;

        bool operator<(const Label& other) const {
            return location < other.location;
        }

        bool operator==(const Label& other) const {
            return location == other.location;
        }

        bool operator>(const Label& other) const {
            return location > other.location;
        }

        bool operator>=(const Label& other) const {
            return location >= other.location;
        }

        bool operator<(const Location& location) const {
            return this->location < location;
        }

        bool operator>(const Location& location) const {
            return this->location > location;
        }

        bool operator==(const Location& location) const {
            return this->location == location;
        }
    };

    std::vector<Label> labels;
};


#endif // HAD_XLR8_UNNAMED_LABEL_LIST_H
