/*
ChecksumBody.cc --

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

#include "ChecksumBody.h"
#include "Body.h"
#include "ExpressionParser.h"
#include "ParseException.h"

Body ChecksumBody::parse(Tokenizer& tokenizer) {
    auto algorithm_name = tokenizer.next();
    if (!algorithm_name.is_name_like()) {
        throw ParseException(algorithm_name, "expected name, got " + std::string(algorithm_name.type_name()));
    }
    auto algorithm = ChecksumAlgorithm::create(algorithm_name.as_symbol());
    auto expression_parser = ExpressionParser(tokenizer);
    tokenizer.expect(Token::comma);
    auto start = expression_parser.parse();
    tokenizer.expect(Token::comma);
    auto end = expression_parser.parse();

    auto parameter_names = algorithm->parameter_names();
    auto parameters = std::unordered_map<Symbol, Expression>();

    while (auto token = tokenizer.next()) {
        if (token.is_end_of_line()) {
            break;
        }
        auto parameter_name = tokenizer.expect(Token::NAME);
        auto parameter = expression_parser.parse();

        if (!parameter_names.contains(parameter_name.as_symbol())) {
            throw ParseException(parameter_name, "unknown parameter" + parameter_name.as_string() + " for algorithm " + algorithm_name.as_string());
        }
        parameters[parameter_name.as_symbol()] = parameter;
    }

    return Body(std::make_shared<ChecksumBody>(algorithm, start, end, parameters));
}

void ChecksumBody::encode(std::string& bytes, const Memory* memory) const {
    for (auto i = 0; i < algorithm->result_size(); i++) {
        bytes += '\0';
    }
}

std::optional<Body> ChecksumBody::evaluated(const EvaluationContext& context) const {
    auto new_start = start.evaluated(context);
    auto new_end = end.evaluated(context);
    auto changed = new_start || new_end;
    auto new_parameters = parameters;
    auto fully_evaluated = new_start.value_or(start).has_value() && new_end.value_or(end).has_value();
    for (auto& [name, expression] : parameters) {
        if (auto new_expression = expression.evaluated(context)) {
            new_parameters[name] = *new_expression;
            if (!new_expression->has_value()) {
                fully_evaluated = false;
            }
            changed = true;
        }
        else {
            if (!expression.has_value()) {
                fully_evaluated = false;
            }
        }
    }

    if (context.type == EvaluationContext::OUTPUT) {
        if (!context.conditional && context.offset.has_size() && fully_evaluated) {
            auto parameter_values = std::unordered_map<Symbol, Value>();
            for (auto& [name, expression] : new_parameters) {
                parameter_values[name] = *expression.value();
            }
            context.result.checksums.emplace_back(algorithm, *context.offset.size(), new_start.value_or(start).value()->unsigned_value(), new_end.value_or(end).value()->unsigned_value(), parameter_values);
            changed = true;
        }
        else {
            throw Exception("unknown values in .ckecksum");
        }
    }

    if (changed) {
        return Body(std::make_shared<ChecksumBody>(algorithm, new_start.value_or(start), new_end.value_or(end), new_parameters));
    }
    else {
        return {};
    }
}


void ChecksumBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".checksum " << algorithm->name << ", " << start << ", " << end;
    // TODO: parameters
    stream << std::endl;
}
