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

#include <tpau-cpp-kernal/LocationException.h>

#include "ChecksumBody.h"
#include "Body.h"
#include "ExpressionParser.h"

using namespace tpau::cpp_kernal;

Body ChecksumBody::parse(Tokenizer& tokenizer) {
    auto algorithm_name = tokenizer.next();
    if (!algorithm_name.is_name_like()) {
        throw LocationException(algorithm_name.location, "expected name, got {}", algorithm_name.type_name());
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
            throw LocationException(parameter_name.location, "unknown parameter {} for algorithm {}", parameter_name.as_string(), algorithm_name.as_string());
        }
        parameters[parameter_name.as_symbol()] = parameter;
    }

    return Body(std::make_shared<ChecksumBody>(algorithm, start, end, parameters));
}

void ChecksumBody::encode(std::string& bytes, const Memory* memory) const {
    for (size_t i = 0; i < algorithm->result_size(); i++) {
        bytes += '\0';
    }
}

std::optional<Body> ChecksumBody::evaluate_process(const EvaluationContext& context) {
    auto fully_evaluated = start.has_value() && end.has_value() && std::all_of(parameters.begin(), parameters.end(), [](const auto& pair) {return pair.second.has_value();});

    if (fully_evaluated) {
        if (!context.conditional && context.offset.has_size() && fully_evaluated) {
            auto parameter_values = std::unordered_map<Symbol, Value>();
            for (auto& [name, expression] : parameters) {
                parameter_values[name] = *expression.value();
            }
            context.result.checksums.emplace_back(algorithm, *context.offset.size(), start.value()->unsigned_value(), end.value()->unsigned_value(), parameter_values);
        }
        else {
            throw LocationException(start.location(), "unknown values in .checksum");
        }
    }

    return {};
}


void ChecksumBody::serialize(std::ostream& stream, const std::string& prefix) const {
    stream << prefix << ".checksum " << algorithm->name << ", " << start << ", " << end;
    // TODO: parameters
    stream << std::endl;
}


void ChecksumBody::traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    expression_callable(start);
    expression_callable(end);
    for (auto& [name, expression] : parameters) {
        expression_callable(expression);
    }
}
