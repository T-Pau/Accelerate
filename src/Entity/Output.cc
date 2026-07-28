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

#include "Entity/Output.h"

#include "Assembler.h"
#include "Target.h"

Token Output::token_output = {Token::NAME, ".output"};

Output::Output(const Location& location, const Target* target, Body body) : ScopeEntity(location, token_output.as_symbol(), Visibility::PUBLIC, target->file_scope(), false), body(std::move(body)) {
    add_memory_constant(Assembler::token_data_start.as_symbol(), MemoryInfoExpression::InfoType::MEMORY_START);
    add_memory_constant(Assembler::token_data_end.as_symbol(), MemoryInfoExpression::InfoType::MEMORY_END);
}

void Output::add_memory_constant(Symbol name, MemoryInfoExpression::InfoType info_type) {
    auto constant = std::make_shared<Constant>(Location(), name, Visibility::ENTITY, scope(), false, MemoryInfoExpression::create(Location(), this, info_type));
    add(constant);
}

void Output::traverse(std::function<void(Entity&)> entity_callback, std::function<void(Body&)> body_callback, std::function<void(Expression&)> expression_callback) {
    ScopeEntity::traverse(entity_callback, body_callback, expression_callback);
    body_callback(body);
}

void Output::resolve_implementation() {
    resolve_constants();
    body.resolve(scope().get(), this);
}
