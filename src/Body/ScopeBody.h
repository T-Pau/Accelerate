#ifdef IN_XLR8_SCOPE_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_SCOPE_BODY_H
#ifndef HAD_XLR8_SCOPE_BODY_H
#define HAD_XLR8_SCOPE_BODY_H

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

#include <tpau-cpp-kernal/Exception.h>

#include "Body/Body.h"
#include "Entity/Constant.h"
#include "Scope.h"

using namespace tpau::cpp_kernal;

/**
 * @brief Represents a body, that introduces a new scope for its contained body.  */
class ScopeBody : public BodyElement {
  public:
    /**
     * @brief Create a new ScopeBody.
     *
     * @param containing_scope The scope containing the ScopeBody.
     * @param body The body to wrap.
     * @return The ScopeBody.
     */
    static Body create(const std::shared_ptr<Scope>& containing_scope, Body body) { return Body(std::make_shared<ScopeBody>(containing_scope, std::move(body))); }

    /**
     * @brief Construct a new ScopeBody.
     *
     * @param containing_scope The scope containing the new scope.
     * @param body The body to wrap.
     */
    ScopeBody(const std::shared_ptr<Scope>& containing_scope, Body body) : inner_scope_(std::make_shared<Scope>(Visibility::ARGUMENT, containing_scope)), body(std::move(body)) {}

    /**
     * @brief Get the inner scope of the ScopeBody.
     *
     * @return The inner_scope.
     */
    [[nodiscard]] std::shared_ptr<Scope> inner_scope() const { return inner_scope_; }

    /**
     * @brief Append a body to the contained body.
     *
     * @param inner_body The body to append.
     */
    void append(Body inner_body) { body.append(std::move(inner_body)); }

    [[nodiscard]] SizeRange size_range() const override { return body.size_range(); }

    [[nodiscard]] SizeRange offset() const override { return body.offset(); }

    [[nodiscard]] std::shared_ptr<BodyElement> clone() const override { throw Exception("can't clone ScopeBody"); }

    [[nodiscard]] bool empty() const override { return body.empty(); }

    void encode(std::string& bytes, const Memory* memory) override { body.encode(bytes, memory); }

    void traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) override;
    void resolve(Scope* scope, Entity* containing_entity) override;

    /**
     * @brief Evaluate the body element after all sub-body-elements and sub-expressions have been evaluated.
     *
     * Returns the contained body if it is fully evaluated, {} otherwise. Sub-classes should override this method if the scope cannot be discarded once the contained body is fully evaluated.
     *
     * @param context The evaluation context.
     * @return The Body to replace this BodyElement with, {} if no replacement is needed.
     */
    [[nodiscard]] std::optional<Body> evaluate_process(const EvaluationContext& context) override;

    void serialize(std::ostream& stream, const std::string& prefix) const override;

    [[nodiscard]] bool fully_evaluated() override { return body.fully_evaluated(); }

    void add(std::shared_ptr<Constant> constant);

  protected:
    /// @brief The scope introduced by the ScopeBody.
    std::shared_ptr<Scope> inner_scope_;

    std::unordered_set<std::shared_ptr<Constant>> constants;

    /// @brief The body contained within the ScopeBody.
    Body body;
};

#endif // HAD_XLR8_SCOPE_BODY_H
#undef IN_XLR8_SCOPE_BODY_H
