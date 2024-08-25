/*
EvaluationResult.cc --

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

#ifndef EVALUATION_RESULT_H
#define EVALUATION_RESULT_H

#include <unordered_set>

#include "ChecksumComputation.h"
#include "Symbol.h"
#include "UsedEntities.h"

class LabelExpression;
class Object;


class EvaluationResult {
public:
    EvaluationResult() = default;

    void add_unresolved_function(Symbol name) {unresolved_functions.insert(name);}
    void add_unresolved_macro(Symbol name) {unresolved_macros.insert(name);}
    void add_unresolved_variable(Symbol name) {unresolved_variables.insert(name);}

    uint64_t next_unnamed_label{1};
    std::unordered_set<Symbol> unresolved_functions;
    std::unordered_set<Symbol> unresolved_macros;
    std::unordered_set<Symbol> unresolved_variables;
    std::unordered_set<Object*> used_objects;
    UsedEntities used_entites;
    std::vector<ChecksumComputation> checksums;
};

#endif // EVALUATION_RESULT_H
