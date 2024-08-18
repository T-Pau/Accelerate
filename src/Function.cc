/*
Function.cc --

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

#include "Function.h"
#include "ParseException.h"
#include "SequenceTokenizer.h"

#define DEFINITION "definition"

bool Function::initialized = false;
Token Function::token_definition;

void Function::initialize() {
    if (!initialized) {
        initialized = true;
        token_definition = Token(Token::NAME, DEFINITION);
    }
}

Function::Function(ObjectFile* owner, const Token& name, const std::shared_ptr<ParsedValue>& definition_) : Callable(owner, name, definition_) {
    auto parameters = definition_->as_dictionary();
    initialize();
    auto tokenizer = SequenceTokenizer((*parameters)[token_definition]->as_scalar()->tokens);
    definition = Expression(tokenizer);
    if (!tokenizer.ended()) {
        throw ParseException(tokenizer.current_location(), "invalid definition");
    }
}


std::ostream& operator<<(std::ostream& stream, const Function& function) {
    function.serialize(stream);
    return stream;
}


Expression Function::call(const Location& location, const std::vector<Expression>& arguments) const {
    EvaluationResult result;
    // TODO: use location
    return definition.evaluated(EvaluationContext(result, EvaluationContext::ARGUMENTS, bind(arguments))).value_or(definition);
}


void Function::serialize(std::ostream& stream) const {
    stream << ".function " << name << " {" << std::endl;
    serialize_callable(stream);
    stream << "    " DEFINITION ": " << definition << std::endl;
    stream << "}" << std::endl;
}
