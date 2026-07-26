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

#include "Body.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>

#include "BlockBody.h"
#include "EmptyBody.h"
#include "Entity/Entity.h"
#include "ScopeBody.h"

#include <type_traits>

using namespace tpau::cpp_kernal;

namespace {

template <typename Callable> auto handle_translation_errors(BodyElement& element, Callable&& callable, Entity* containing_entity = nullptr) -> std::conditional_t<std::is_void_v<std::invoke_result_t<Callable>>, void, std::invoke_result_t<Callable>> {
    using Result = std::invoke_result_t<Callable>;

    if constexpr (std::is_void_v<Result>) {
        if (!element.valid) {
            return;
        }
        auto result = DiagnosticOutput::global.log_exceptions(callable);
        if (!result) {
            if (containing_entity && DiagnosticOutput::global.verbose_error_messages) {
                std::cerr << *containing_entity << std::endl;
            }
            element.valid = false;
        }
    }
    else {
        if (!element.valid) {
            if (DiagnosticOutput::global.verbose_error_messages) {
                std::cerr << element << std::endl;
            }
            return Result{};
        }
        auto result = DiagnosticOutput::global.log_exceptions(callable);
        if (!result) {
            element.valid = false;
            return Result{};
        }
        return *result;
    }
}

} // namespace

Body::Body() : element(std::make_shared<EmptyBody>()) {}

Body::Body(const std::shared_ptr<BodyElement>& new_element) {
    if (new_element) {
        element = new_element;
    }
    else {
        element = std::make_shared<EmptyBody>();
    }
}

void Body::append(const Body& new_element) {
    if (new_element.empty()) {
        return;
    }
    if (element->empty()) {
        element = new_element.element;
    }
    else {
        const auto [success, new_body] = append_sub(new_element);
        if (success) {
            if (new_body) {
                element = new_body->element;
            }
        }
        else {
            element = std::make_shared<BlockBody>(std::vector<Body>({*this, new_element}));
        }
    }
}

void Body::evaluate(const EvaluationContext& context) {
    TRACE_BEGIN_INSTANCE(element, "evaluating", "new offset: {}, previous offset: {}, size: {}", context.offset, element->offset(), element->size_range());
    handle_translation_errors(
        *element,
        [&] {
            if (const auto new_body = element->evaluate(context)) {
                *this = *new_body;
            }
            // We set the offset after evaluation so the evaluated body can check for changes in offset.
            element->offset_ = context.offset;
        },
        context.entity);
    TRACE_END_INSTANCE(element, "evaluating", "offset: {}, size: {}", element->offset(), element->size_range());
}

std::optional<Body> Body::back() const {
    if (const auto block = as<BlockBody>()) {
        return block->back();
    }
    else if (!empty()) {
        return *this;
    }
    else {
        return {};
    }
}

std::pair<bool, std::optional<Body>> Body::append_sub(const Body& new_element) { return element->append_sub(*this, new_element); }

std::ostream& operator<<(std::ostream& stream, const Body& body) {
    body.serialize(stream, "    ");
    return stream;
}

void Body::resolve(Scope* scope, Entity* containing_entity) {
    handle_translation_errors(*element, [&] { element->resolve(scope, containing_entity); }, containing_entity);
}

void Body::expand_calls() {
    // TODO: pass containing_entity to handle_translation_errors
    handle_translation_errors(*element, [&] { element->expand_calls(); });
}

void Body::enter_names(Scope* scope, Entity* containing_entity) {
    TRACE_BEGIN_INSTANCE(element, "entering names", "");
    // TODO: pass containing_entity to handle_translation_errors
    handle_translation_errors(*element, [&] { element->enter_names(scope, containing_entity); });
    TRACE_END_INSTANCE(element, "entering names", "");
}

bool Body::fully_evaluated() {
    return handle_translation_errors(*element, [&] { return element->fully_evaluated(); });
}
