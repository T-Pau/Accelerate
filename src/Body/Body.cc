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
#include "ScopeBody.h"

#include <type_traits>
#include <utility>

using namespace tpau::cpp_kernal;

namespace {

template<typename Callable>
auto handle_translation_errors(BodyElement& element, Callable&& callable) {
    using Result = std::invoke_result_t<Callable>;

    try {
        if constexpr (std::is_void_v<Result>) {
            if (!element.valid) {
                return;
            }
            std::forward<Callable>(callable)();
        }
        else {
            if (!element.valid) {
                return Result{};
            }
            return std::forward<Callable>(callable)();
        }
    }
    catch (LocationException &ex) {
        DiagnosticOutput::global.error(ex);
        element.valid = false;
    }
    catch (Exception &ex) {
        DiagnosticOutput::global.error(element.location, ex);
        element.valid = false;
    }
    catch (std::exception &ex) {
        DiagnosticOutput::global.error(element.location, "unexpected error: %s", ex.what());
        element.valid = false;
    }

    if constexpr (!std::is_void_v<Result>) {
        return Result{};
    }
}

} // namespace

Body::Body(): element(std::make_shared<EmptyBody>()) {}

Body::Body(const std::shared_ptr<BodyElement> &new_element) {
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
        if (const auto new_body = append_sub(new_element)) {
            element = new_body->element;
        }
        else {
            element = std::make_shared<BlockBody>(std::vector<Body>({*this, new_element}));
        }
    }
}

void Body::evaluate(const EvaluationContext& context) {
    handle_translation_errors(*element, [&] {
        if (const auto new_body = element->evaluate(context)) {
            *this = *new_body;
        }
    });
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

std::optional<Body> Body::append_sub(const Body& new_element) {
    return element->append_sub(*this, new_element);
}


Body Body::scoped(const std::shared_ptr<Scope>& inner_environment) const {
    if (is<ScopeBody>() && !inner_environment) {
        return *this;
    }
    else {
        return ScopeBody::create(*this, inner_environment);
    }
}

std::ostream& operator<<(std::ostream& stream, const Body& body) {
    body.serialize(stream, "    ");
    return stream;
}


void Body::resolve(Scope* scope, Entity* containing_entity) {
    handle_translation_errors(*element, [&] {
        element->resolve(scope, containing_entity);
    });
}


void Body::expand_calls() {
    handle_translation_errors(*element, [&] {
        element->expand_calls();
    });
}

void Body::enter_names(Scope* scope, Entity* containing_entity) {
    handle_translation_errors(*element, [&] {
        element->enter_names(scope, containing_entity);
    });
}