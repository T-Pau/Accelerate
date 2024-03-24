/*
Entity.h --

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

#ifndef ENTITY_H
#define ENTITY_H

#include "ParsedValue.h"
#include "Unresolved.h"

class Macro;
class Object;

class Entity {
  public:
    Entity(ObjectFile* owner, const Token& name, const std::shared_ptr<ParsedValue>& definition);
    Entity(ObjectFile* owner, const Token& name, Visibility visibility, bool default_only);
    virtual ~Entity() = default;

    [[nodiscard]] Macro* as_macro();
    [[nodiscard]] Object* as_object();
    void evaluate();
    [[nodiscard]] bool check_unresolved(Unresolved& unresolved) const;
    [[nodiscard]] bool is_default_only() const {return default_only;}
    [[nodiscard]] bool is_macro() {return as_macro();}
    [[nodiscard]] bool is_object() {return as_object();}
    void set_owner(ObjectFile* new_owner);

    // TODO: make protected once BodyParser doesn't evaluate body directly.
    void process_result(EvaluationResult& result);

    Token name;
    Visibility visibility;
    ObjectFile* owner = nullptr;
    std::unordered_set<Object*> referenced_objects;
    std::unordered_set<Symbol> unresolved_functions;
    std::unordered_set<Symbol> unresolved_macros;
    std::unordered_set<Symbol> unresolved_variables;
    std::shared_ptr<Environment> environment;

  protected:
    void serialize_entity(std::ostream& stream) const;

    [[nodiscard]] virtual EvaluationContext evaluation_context(EvaluationResult& result) {return EvaluationContext(result, this);}
    virtual void evaluate_inner(EvaluationContext& context) = 0;

  private:
    [[nodiscard]] bool check_unresolved(const std::unordered_set<Symbol>& unresolved, Unresolved::Part& part) const;

    static const Token token_visibility;

    bool default_only;
};


#endif // ENTITY_H
