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

#include "Expression/FunctionExpression.h"

#include "Expression/DefinedExpression.h"
#include "Expression/ExistsExpression.h"
#include "Expression/FillExpression.h"
#include "Expression/InRangeExpression.h"
#include "Expression/MinMaxExpression.h"
#include "Expression/SizeofExpression.h"
#include "EvaluationContext.h"
#include "Expression.h"
#include "ObjectFileParser.h"

// clang-format off
const std::unordered_map<Symbol, Expression (*)(const Location& location, const std::vector<Expression>&)> FunctionExpression::builtin_functions = {
    {Symbol(".defined"), &DefinedExpression::create},
    {Symbol{".exists"}, &ExistsExpression::create},
    {Symbol{".fill"}, &FillExpression::create},
    {Symbol{".max"}, &MinMaxExpression::create_max},
    {Symbol{".min"}, &MinMaxExpression::create_min},
    {Symbol(".sizeof"), &SizeofExpression::create}
};
// clang-format on

void FunctionExpression::serialize_sub(std::ostream& stream) const {
    stream << name << "(";
    auto first = true;
    for (auto& argument : arguments) {
        if (!first) {
            stream << ", ";
        }
        stream << argument;
        first = false;
    }
    stream << ")";
}

void FunctionExpression::setup(FileTokenizer& tokenizer) {
    for (const auto& pair : builtin_functions) {
        tokenizer.add_literal(Token{Token::Type::NAME, Location(), pair.first});
    }
}

Expression FunctionExpression::create(const Location& location, Symbol name, const std::vector<Expression>& arguments) {
    if (name == ObjectFileParser::token_in_range.as_symbol()) {
        return InRangeExpression::create(location, arguments);
    }
    else if (name == ObjectFileParser::token_label_offset.as_symbol()) {
        // TODO: create LabelOffsetExpression
        // return LabelOffsetExpression::create(location, arguments);
    }
    const auto it = builtin_functions.find(name);
    if (it != builtin_functions.end()) {
        return it->second(location, arguments);
    }
    return Expression(std::make_shared<FunctionExpression>(location, name, arguments));
}

    


void FunctionExpression::resolve(Scope* scope, Entity* containing_entity) {
    BaseExpression::resolve(scope, containing_entity);
    function = scope->get_function(name);
    if (!function) {
        throw LocationException(location, "function {} not found", name);
    }
}

void FunctionExpression::expand_calls() {
    for (auto& argument : arguments) {
        argument.expand_calls();
    }

    // TODO: copy function body into this expression, replacing arguments with their values
}

void FunctionExpression::traverse(std::function<void(Expression&)> callable) {
    for (auto& argument : arguments) {
        callable(argument);
    }
}
