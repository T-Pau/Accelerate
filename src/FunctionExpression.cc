/*
FunctionExpression.cc --

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

#include "FunctionExpression.h"

#include <complex>

#include "DefinedExpression.h"
#include "ExistsExpression.h"
#include "EvaluationContext.h"
#include "Expression.h"
#include "FillExpression.h"
#include "Function.h"
#include "InRangeExpression.h"
#include "LabelExpression.h"
#include "ObjectFileParser.h"
#include "SizeofExpression.h"

const std::unordered_map<Symbol, Expression (*)(const std::vector<Expression>&)> FunctionExpression::builtin_functions = {
    {Symbol(".defined"), &DefinedExpression::create},
    {Symbol{".exists"}, &ExistsExpression::create},
    {Symbol{".fill"}, &FillExpression::create},
    {Symbol(".sizeof"), &SizeofExpression::create}
};

void FunctionExpression::serialize_sub(std::ostream &stream) const {
    stream << name << "(";
    auto first = true;
    for (auto& argument: arguments) {
        if (!first) {
            stream << ", ";
        }
        stream << argument;
        first = false;
    }
    stream << ")";
}

void FunctionExpression::collect_objects(std::unordered_set<Object*>& objects) const {
    for (auto& argument : arguments) {
        argument.collect_objects(objects);
    }
}

void FunctionExpression::setup(FileTokenizer& tokenizer) {
    for (const auto& pair: builtin_functions) {
        tokenizer.add_literal(Token{Token::Type::NAME, Location(), pair.first});
    }
}

Expression FunctionExpression::create(Symbol name, const std::vector<Expression> &arguments) {
    if (name == ObjectFileParser::token_in_range.as_symbol()) {
        return InRangeExpression::create(arguments);
    }
    else if (name == ObjectFileParser::token_label_offset.as_symbol()) {
        return LabelExpression::create(arguments);
    }
    const auto it = builtin_functions.find(name);
    if (it != builtin_functions.end()) {
        return it->second(arguments);
    }
    return Expression(std::make_shared<FunctionExpression>(name, arguments));
}

std::optional<Expression> FunctionExpression::evaluated(const EvaluationContext& context) const {
    std::vector<Expression> new_arguments;
    auto changed = false;
    for (auto& argument: arguments) {
        if (const auto new_argument = argument.evaluated(context)) {
            new_arguments.emplace_back(*new_argument);
            changed = true;
        }
        else {
            new_arguments.emplace_back(argument);
        }
    }

    if (const auto function = context.environment->get_function(name)) {
        return function->call(new_arguments);
    }
    else {
        context.result.add_unresolved_function(name);
        if (changed) {
            return Expression(name, new_arguments);
        }
        else {
            return {};
        }
    }
}
