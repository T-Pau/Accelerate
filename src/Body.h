/*
Body.h --

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

#ifndef BODY_H
#define BODY_H

#include "BodyElement.h"
#include "EvaluationContext.h"
#include "Label.h"
#include "Location.h"
#include "SizeRange.h"
#include "Visibility.h"

class BlockBody;
class DataBody;
class DataBodyElement;
class ErrorBody;
class IfBodyClause;
class LabelBody;

class Body {
public:

    Body();
    explicit Body(const std::shared_ptr<BodyElement>& element);
    // Assignment
    Body(Visibility visibility, Symbol name, Expression value);
    // Block
    explicit Body(const std::vector<Body>& elements);
    // Data
    explicit Body(std::vector<DataBodyElement> elements);
    // Error
    Body(Location location, std::string message);
    // If
    explicit Body(const std::vector<IfBodyClause>& clauses);
    // Label
    explicit Body(std::shared_ptr<Label> label);
    // Macro
    Body(Token name, std::vector<Expression> arguments);
    // Memory
    Body(Expression bank, Expression start_address, Expression end_address);

    [[nodiscard]] BlockBody* as_block() const;
    [[nodiscard]] DataBody* as_data() const;
    [[nodiscard]] ErrorBody* as_error() const;
    [[nodiscard]] LabelBody* as_label() const;
    void append(const Body& element);
    std::optional<Body> append_sub(Body element);
    [[nodiscard]] std::optional<Body> back() const;
    void collect_objects(std::unordered_set<Object*>& objects) const {element->collect_objects(objects);}
    [[nodiscard]] bool empty() const {return element->empty();}
    [[nodiscard]] Body make_unique() const;
    void encode(std::string& bytes, const Memory* memory = nullptr) const {element->encode(bytes, memory);}
    bool evaluate(EvaluationResult& result, Object* object, std::shared_ptr<Environment> environment = {}, SizeRange offset = SizeRange(), bool conditional = false, bool shallow = false) {return evaluate(EvaluationContext(result, object, environment, offset, conditional, shallow));}
    bool evaluate(const EvaluationContext& context);
    [[nodiscard]] std::optional<Body> evaluated (const EvaluationContext& context) const;
    [[nodiscard]] bool is_block() const {return as_block() != nullptr;}
    [[nodiscard]] bool is_data() const {return as_data() != nullptr;}
    [[nodiscard]] bool is_error() const {return as_error() != nullptr;}
    [[nodiscard]] bool is_label() const {return as_label() != nullptr;}
    [[nodiscard]] Body scoped() const;
    void serialize(std::ostream& stream, const std::string& prefix = "") const {element->serialize(stream, prefix);}
    [[nodiscard]] std::optional<uint64_t> size() const {return element->size();}
    [[nodiscard]] SizeRange size_range() const {return element->size_range();}
    [[nodiscard]] long use_count() const {return element.use_count();}

private:
    std::shared_ptr<BodyElement> element;


};

#endif // BODY_H
