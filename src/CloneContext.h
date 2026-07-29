#ifdef IN_XLR8_CLONE_CONTEXT_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_CLONE_CONTEXT_H
#ifndef HAD_XLR8_CLONE_CONTEXT_H
#define HAD_XLR8_CLONE_CONTEXT_H

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

#include <memory>
#include <unordered_map>

class Constant;
class Scope;

class CloneContext {
  public:
    CloneContext() = default;

    CloneContext(std::shared_ptr<Scope> containing_scope, const CloneContext* parent = nullptr) : containing_scope(std::move(containing_scope)), parent(parent) {}

    std::shared_ptr<Constant> map(const std::shared_ptr<Constant>& constant) const;

    void add_mapping(const std::shared_ptr<Constant>& original, const std::shared_ptr<Constant>& clone) { constant_map[original] = clone; }

    std::shared_ptr<Scope> containing_scope;

  private:
    const CloneContext* parent = nullptr;
    std::unordered_map<std::shared_ptr<Constant>, std::shared_ptr<Constant>> constant_map;
};

#endif // HAD_XLR8_CLONE_CONTEXT_H
#undef IN_XLR8_CLONE_CONTEXT_H
