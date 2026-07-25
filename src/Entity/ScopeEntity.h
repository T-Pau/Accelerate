#ifdef IN_XLR8_SCOPE_ENTITY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_SCOPE_ENTITY_H
#ifndef HAD_XLR8_SCOPE_ENTITY_H
#define HAD_XLR8_SCOPE_ENTITY_H

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

#include "Entity/Entity.h"

class Scope;

class ScopeEntity : public Entity {
  public:
    ScopeEntity(const Location& location, Symbol name, std::shared_ptr<Scope> containing_scope, const std::shared_ptr<StructuredValue>& definition);

    ScopeEntity(const Location& location, Symbol name, Visibility visibility, std::shared_ptr<Scope> containing_scope, bool default_only = false);

    [[nodiscard]] std::shared_ptr<Scope> scope() const { return scope_; }

    void add(std::shared_ptr<Constant> constant);

  protected:
    std::unordered_set<std::shared_ptr<Constant>> constants;
    std::shared_ptr<Scope> scope_;
};

#endif // HAD_XLR8_SCOPE_ENTITY_H
#undef IN_XLR8_SCOPE_ENTITY_H
