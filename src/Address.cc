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

#include "Address.h"

#include <iomanip>

#include <tpau-cpp-kernal/Int.h>
#include <tpau-cpp-kernal/LocationException.h>

#include "ExpressionParser.h"

using namespace tpau::cpp_kernal;

Symbol Address::bank_name{"bank"};
Symbol Address::address_name{"address"};

Address::Address(Tokenizer& tokenizer) : bank_component(bank_name, 0), address_component(address_name, 0) {
    auto expression = ExpressionParser(tokenizer).parse();
    auto token = tokenizer.next();

    if (token == Token::colon) {
        bank_component = Component(bank_name, std::move(expression));
        expression = ExpressionParser(tokenizer).parse();
    }
    else {
        tokenizer.unget(token);
    }
    address_component = Component(address_name, std::move(expression));

    evaluate_process();
}

std::ostream& operator<<(std::ostream& stream, Address address) {
    address.serialize(stream);
    return stream;
}

void Address::serialize(std::ostream& stream) const {
    if (!bank_component.has_value() || bank_component.value() != 0) {
        stream << bank_component << ":";
    }
    stream << address_component;
}

void Address::resolve(Scope* scope, Entity* containing_entity) {
    bank_component.resolve(scope, containing_entity);
    address_component.resolve(scope, containing_entity);

    evaluate_process();
}

void Address::evaluate(const EvaluationContext& context) {
    bank_component.evaluate(context);
    address_component.evaluate(context);

    evaluate_process();
}

void Address::evaluate_process() {
    bank_component.evaluate_process();
    address_component.evaluate_process();
}

void Address::Component::resolve(Scope* scope, Entity* containing_entity) {
    if (std::holds_alternative<Expression>(component_value)) {
        std::get<Expression>(component_value).resolve(scope, containing_entity);
    }
}

void Address::Component::evaluate(const EvaluationContext& context) {
    if (std::holds_alternative<Expression>(component_value)) {
        std::get<Expression>(component_value).evaluate(context);
    }
}

void Address::Component::evaluate_process() {
    if (std::holds_alternative<Expression>(component_value)) {
        auto& expression = std::get<Expression>(component_value);
        auto type = expression.type();
        if (type.has_value() && type != Value::Type::UNSIGNED) {
            throw LocationException(expression.location(), "{} must be unsigned integer", name);
        }
        if (expression.has_value()) {
            component_value = expression.value()->unsigned_value();
        }
    }
}

void Address::Component::serialize(std::ostream& stream, bool output_if_zero) const {
    if (std::holds_alternative<uint64_t>(component_value)) {
        auto value = std::get<uint64_t>(component_value);
        if (!output_if_zero && value == 0) {
            return;
        }
        stream << "$" << std::setfill('0') << std::setw(std::max(static_cast<int>(Int::minimum_byte_size(std::get<uint64_t>(component_value))), 2)) << std::hex << std::get<uint64_t>(component_value) << std::dec;
    }
    else if (std::holds_alternative<Expression>(component_value)) {
        stream << std::get<Expression>(component_value);
    }
}

bool Address::operator<(const Address& other) const {
    if (bank_component.has_value() && other.bank_component.has_value()) {
        if (bank_component.value() != other.bank_component.value()) {
            return bank_component.value() < other.bank_component.value();
        }
    }
    else if (bank_component.has_value()) {
        return false;
    }
    else if (other.bank_component.has_value()) {
        return true;
    }

    if (address_component.has_value() && other.address_component.has_value()) {
        return address_component.value() < other.address_component.value();
    }
    else if (address_component.has_value()) {
        return false;
    }
    else if (other.address_component.has_value()) {
        return true;
    }

    return false;
}

bool operator<(const std::optional<Address>& a, const std::optional<Address>& b) {
    if (a.has_value() && b.has_value()) {
        return *a < *b;
    }
    else if (a.has_value()) {
        return false;
    }
    else if (b.has_value()) {
        return true;
    }
    else {
        return false;
    }
}

bool Address::operator==(const Address& other) const {
    if (bank_component.has_value() != other.bank_component.has_value()) {
        return false;
    }
    if (address_component.has_value() != other.address_component.has_value()) {
        return false;
    }
    if (bank_component.has_value() && other.bank_component.has_value()) {
        if (bank_component.value() != other.bank_component.value()) {
            return false;
        }
    }
    if (address_component.has_value() && other.address_component.has_value()) {
        if (address_component.value() != other.address_component.value()) {
            return false;
        }
    }
    return true;
}

bool Address::Component::has_value() const { return std::holds_alternative<uint64_t>(component_value) || std::get<Expression>(component_value).has_value(); }

std::optional<uint64_t> Address::Component::value() const {
    if (std::holds_alternative<uint64_t>(component_value)) {
        return std::get<uint64_t>(component_value);
    }
    auto& expression = std::get<Expression>(component_value);
    if (expression.has_value()) {
        return expression.value()->unsigned_value();
    }
    else {
        return {};
    }
}

std::optional<uint64_t> Address::Component::minimum() const {
    if (std::holds_alternative<uint64_t>(component_value)) {
        return std::get<uint64_t>(component_value);
    }
    auto minimum_value = std::get<Expression>(component_value).minimum_value();
    if (minimum_value) {
        return minimum_value->unsigned_value();
    }
    else {
        return {};
    }
}

std::optional<uint64_t> Address::Component::maximum() const {
    if (std::holds_alternative<uint64_t>(component_value)) {
        return std::get<uint64_t>(component_value);
    }
    auto maximum_value = std::get<Expression>(component_value).maximum_value();
    if (maximum_value) {
        return maximum_value->unsigned_value();
    }
    else {
        return {};
    }
}

std::ostream& operator<<(std::ostream& stream, Address::Component component) {
    component.serialize(stream);
    return stream;
}
