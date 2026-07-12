#ifdef IN_XLR8_BODY_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_BODY_H
#ifndef HAD_XLR8_BODY_H
#define HAD_XLR8_BODY_H

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

#include <tpau-cpp-kernal/Location.h>

#include "BodyElement.h"
#include "EvaluationContext.h"
#include "SizeRange.h"

using namespace tpau::cpp_kernal;


/**
 * @brief Represents a body of an object or macro. This is the class other parts of the program interact with. It is a wrapper around the BodyElement hierarchy.
 */
class Body {
public:

    /**
     * @brief Constructs an empty body.
     */
    Body();

    /**
     * @brief Constructs a body with a given element.
     *
     * @param element The element to wrap.
     */
    explicit Body(const std::shared_ptr<BodyElement>& element);

    /**
     * Get the body element as a specific type.
     * 
     * @tparam T The type to cast to.
     * @return The body element as type T, or nullptr if it is not of that type.
     */
    template <typename T>
    [[nodiscard]] T* as() const {
        return dynamic_cast<T*>(element.get());
    }

    /**
     * @brief Check if the body element is of a specific type.
     *
     * @tparam T The type to check against.
     * @return `true` if the body element is of type T, `false` otherwise.
     */
    template<typename T>
    [[nodiscard]] bool is() const {
        return as<T>() != nullptr;
    }

    /**
     * @brief Get the body element.
     *
     * @return The body element as a shared pointer.
     */
    [[nodiscard]] std::shared_ptr<BodyElement> get_element() const {return element;}

    /**
     * @brief Appends a new element to the body.
     *
     * If necessary, the body will be converted to a BlockBody to accommodate the new element.
     *
     * @param element The element to append.
     */
    void append(const Body& element);

    void enter_names(Scope* scope, Entity* containing_entity);

    /**
     * Resolve all names in the body.
     *
     * @param scope The scope to resolve names in.
     * @param containing_entity The entity that contains the body.
     */
    void resolve(Scope* scope, Entity* containing_entity);

    /**
     * @brief Expand macro and function calls in the body.
     *
     * This will recursively expand all macro and function calls within the body.
     */
    void expand_calls();

    [[nodiscard]] std::optional<Body> append_sub(const Body& element);
    [[nodiscard]] std::optional<Body> back() const;

    /**
     * @brief Check if the BodyElement can be discarded.
     *
     * Subclasses must implement this method.
     *
     * @return `true` if the BodyElement is empty, `false` otherwise.
     */
    [[nodiscard]] bool empty() const {return element->empty();}

    void encode(std::string& bytes, const Memory* memory = nullptr) const {element->encode(bytes, memory);}
    void evaluate(const EvaluationContext& context);
    [[nodiscard]] std::optional<Body> evaluated (const EvaluationContext& context) const;
    [[nodiscard]] Body scoped(const std::shared_ptr<Scope>& = {}) const;
    void serialize(std::ostream& stream, const std::string& prefix = "") const {element->serialize(stream, prefix);}
    [[nodiscard]] SizeRange offset() const {return element->offset();}
    [[nodiscard]] std::optional<uint64_t> size() const {return element->size();}
    [[nodiscard]] SizeRange size_range() const {return element->size_range();}

    [[nodiscard]] bool valid() const {return element->valid;}

private:
    std::shared_ptr<BodyElement> element;
};

std::ostream& operator<<(std::ostream& stream, const Body& body);

#endif // HAD_XLR8_BODY_H
#undef IN_XLR8_BODY_H
