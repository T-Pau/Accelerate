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

#include "Expression/Expression.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/LocationException.h>

#include "Expression/EntityExpression.h"
#include "Expression/VoidExpression.h"
#include "Expression/VariableExpression.h"
#include "ExpressionParser.h"

using namespace tpau::cpp_kernal;

std::shared_ptr<BaseExpression> Expression::void_expression = std::make_shared<VoidExpression>(Location());


namespace {
template<typename Callable>
auto handle_translation_errors(BaseExpression& expression, Callable&& callable) {
    using Result = std::invoke_result_t<Callable>;

    try {
        if constexpr (std::is_void_v<Result>) {
            if (!expression.valid) {
                return;
            }
            std::forward<Callable>(callable)();
        }
        else {
            if (!expression.valid) {
                return Result{};
            }
            return std::forward<Callable>(callable)();
        }
    }
    catch (LocationException &ex) {
        if (!ex.empty()) {
            DiagnosticOutput::global.error(ex);
        }
        expression.valid = false;
    }
    catch (Exception &ex) {
        if (!ex.empty()) {
            DiagnosticOutput::global.error(expression.location, ex);
        }
        expression.valid = false;
    }
    catch (std::exception &ex) {
        DiagnosticOutput::global.error(expression.location, "unexpected error: %s", ex.what());
        expression.valid = false;
    }

    if constexpr (!std::is_void_v<Result>) {
        return Result{};
    }
}

} // namespace

Expression::Expression(): expression(void_expression) {}

Expression::Expression(std::shared_ptr<BaseExpression> expression_) {
    if (expression_) {
        expression = std::move(expression_);
    }
    else {
        expression = void_expression;
    }
}


Expression::Expression(Tokenizer& tokenizer) {
    *this = ExpressionParser(tokenizer).parse();
}


#if 0
// We probably won't keep this, but we will move the logic somewhere, so we keep it for reference.
Expression::Expression(const Token& token) {
    if (token == ObjectFileParser::token_object_name) {
        expression = std::make_shared<ObjectNameExpression>(token.location);
    }
    else if (token.is_name() || token == Token::colon_minus || token == Token::colon_plus) {
        expression = std::make_shared<VariableExpression>(token.location, token.as_symbol());
    }
    else {
        expression = std::make_shared<ValueExpression>(token);
    }
}
#endif

void Expression::evaluate(const EvaluationContext& context) {
    auto new_expression = handle_translation_errors(*expression, [&]() {
        return expression->evaluate(context);
    });
    if (new_expression) {
        *this = *new_expression;
    }
}

void Expression::serialize(std::ostream &stream) const {
    if (expression) {
        expression->serialize(stream);
    }
}

Symbol Expression::variable_name() const {
    if (auto variable = as<VariableExpression>()) {
        return variable->symbol;
    }
    if (auto object = as<EntityExpression>()) {
        return object->entity->name;
    }
    return {};
}

std::ostream& operator<<(std::ostream& stream, const Expression& expression) {
    expression.serialize(stream);
    return stream;
}

void Expression::resolve(Scope* scope, Entity* containing_entity) {
    handle_translation_errors(*expression, [&]() {
        expression->resolve(scope, containing_entity);
    });
}

void Expression::expand_calls() {
    handle_translation_errors(*expression, [&]() {
        expression->expand_calls();
    });
}


std::optional<bool> Expression::has_type(Value::Type type) const {
    auto expression_type = this->type();

    if (!expression_type) {
        return {};
    }
    return Value::is_subtype(*expression_type, type);
}
