/*
Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

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

#include "InstructionInvocation.h"

#include <algorithm>

#include <tpau-cpp-kernal/Util.h>

#include "AddressingModeMatcher.h"
#include "Body/ErrorBody.h"
#include "Body/ScopeBody.h"
#include "CPU.h"
#include "Entity/Constant.h"
#include "Expression/BinaryExpression.h"
#include "Expression/ValueExpression.h"
#include "Expression/VariableExpression.h"
#include "ExpressionNode.h"
#include "TokenNode.h"

using namespace tpau::cpp_kernal;

InstructionInvocation::InstructionInvocation(const CPU* cpu, Token name, const std::vector<std::shared_ptr<Node>>& nodes, const std::shared_ptr<Scope>& containing_scope) : nodes(nodes), containing_scope(containing_scope) {
    auto instruction = cpu->instruction(name.as_symbol());
    if (!instruction) {
        throw LocationException(name.location, "Unknown instruction: " + name.as_string());
    }

    location = name.location;
    if (!nodes.empty()) {
        location.extend(nodes.back()->get_location());
    }

    auto matches = cpu->match_addressing_modes(nodes);
    if (matches.empty()) {
        throw LocationException(location, "addressing mode not recognized");
    }

    for (const auto& match : matches) {
        if (instruction->has_addressing_mode(match.addressing_mode)) {
            auto addressing_mode = cpu->addressing_mode(match.addressing_mode);
            auto notation = addressing_mode->notations[match.notation_index];
            variants.emplace_back(Variant{instruction->opcode(match.addressing_mode), *addressing_mode, notation, nodes});
        }
    }

    if (variants.empty()) {
        if (matches.size() == 1) {
            throw LocationException(location, "instruction {} doesn't support addressing mode {}", name, matches.begin()->addressing_mode);
        }
        else {
            auto modes = std::vector<Symbol>();
            for (const auto& match : matches) {
                modes.emplace_back(match.addressing_mode);
            }
            std::ranges::sort(modes);
            throw LocationException(location, "instruction {} doesn't support any of the addressing modes {}", name, join(modes));
        }
    }

    std::erase_if(variants, [](const Variant& variant) { return !variant.valid(); });

    if (variants.empty()) {
        throw LocationException(location, "arguments out of range");
    }

    uses_pc_ = std::any_of(variants.begin(), variants.end(), [](const Variant& variant) { return variant.addressing_mode.get().uses_pc; });
}

Body InstructionInvocation::encode() {
    if (uses_pc() && !pc) {
        throw LocationException(location, "internal error: instruction invocation uses PC but no PC was provided");
    }

    compute_argument_names();

    std::shared_ptr<Scope> scope = containing_scope;
    if (has_unknown_arguments || uses_pc()) {
        scope = std::make_shared<Scope>(Visibility::SCOPE, containing_scope);
        if (uses_pc()) {
            scope->add(std::make_unique<Constant>(Location(), Symbol(".pc"), Visibility::SCOPE, containing_scope, false, *pc));
        }

        for (size_t i = 0; i < argument_names.size(); ++i) {
            const auto& name = argument_names[i];
            const auto& argument = arguments[i];

            if (name) {
                scope->add(std::make_unique<Constant>(Location(), name, Visibility::SCOPE, containing_scope, false, argument));
            }
        }
    }

    std::vector<IfBodyClause> clauses;
    bool have_else = false;

    for (const auto& variant : variants) {
        auto [constraint, body] = variant.encode(scope);

        clauses.emplace_back(constraint, body);

        if (!constraint.has_value()) {
            have_else = true;
            break;
        }
    }
    if (!have_else) {
        clauses.emplace_back(std::optional<Expression>{}, ErrorBody::create(location, "arguments out of range"));
    }

    auto body = IfBody::create(clauses);
    if (scope != containing_scope) {
        body = ScopeBody::create(scope, body);
    }
    return body;
}

void InstructionInvocation::compute_argument_names() {
    argument_names.clear();

    size_t argument_index = 0;
    for (const auto& node : nodes) {
        if (node->type() != Node::EXPRESSION) {
            continue;
        }

        Symbol common_name;
        bool needs_renaming = false;
        bool is_unknown = false;

        for (const auto& variant : variants) {
            auto it = variant.unknown_argument_names.find(argument_index);
            if (it == variant.unknown_argument_names.end()) {
                continue;
            }

            is_unknown = true;

            const auto& name = it->second;

            if (common_name.empty()) {
                common_name = name;
            }
            else if (common_name != name) {
                needs_renaming = true;
                break;
            }
        }

        if (is_unknown) {
            has_unknown_arguments = true;
            if (needs_renaming) {
                common_name = Symbol(std::format(".arg{}", argument_index));
            }
            argument_names.emplace_back(common_name);
        }
        else {
            argument_names.emplace_back();
        }
        arguments.emplace_back(std::dynamic_pointer_cast<ExpressionNode>(node)->expression);

        for (auto& variant : variants) {
            auto it = variant.unknown_argument_names.find(argument_index);
            if (it == variant.unknown_argument_names.end()) {
                continue;
            }

            const auto& name = it->second;

            if (name != common_name) {
                variant.argument_aliases.emplace(name, common_name);
            }
        }

        argument_index += 1;
    }
}

InstructionInvocation::Variant::Variant(uint64_t opcode, const AddressingMode& addressing_mode, const AddressingMode::Notation& notation, const std::vector<std::shared_ptr<Node>>& nodes) : opcode(opcode), addressing_mode(addressing_mode), notation(notation), nodes(nodes) {
    std::unordered_set<Symbol> seen_arguments;

    size_t argument_index = 0;
    for (size_t i = 0; i < notation.elements.size(); ++i) {
        const auto& element = notation.elements[i];
        if (!element.is_argument()) {
            continue;
        }

        const auto& name = element.symbol;
        const auto& node = nodes[i];
        auto argument_definition = addressing_mode.argument(element.symbol);
        auto argument = Argument(argument_definition, node.get());

        seen_arguments.insert(name);

        if (argument.known_invalid()) {
            valid_ = false;
            return;
        }

        arguments.emplace(name, std::move(argument));

        if (node->type() == Node::EXPRESSION) {
            if (!argument.known_value) {
                unknown_argument_names.emplace(argument_index, name);
            }
            argument_index += 1;
        }
    }

    for (const auto& [name, argument_definition] : addressing_mode.arguments) {
        if (!seen_arguments.contains(name)) {
            if (argument_definition->default_value) {
                arguments.emplace(name, Argument(argument_definition.get()));
            }
            else {
                valid_ = false;
                return;
            }
        }
    }
}

std::pair<std::optional<Expression>, Body> InstructionInvocation::Variant::encode(const std::shared_ptr<Scope>& containing_scope) const {
    std::optional<Expression> constraint_expression;
    std::shared_ptr<Scope> scope = std::make_shared<Scope>(Visibility::SCOPE, containing_scope);

    scope->add(std::make_unique<Constant>(Location{}, Symbol(".opcode"), Visibility::SCOPE, containing_scope, false, ValueExpression::create({}, Value(opcode))));

    for (const auto& [name, argument] : arguments) {
        auto renamed_name = get_with_fallback(argument_aliases, name, name);
        if (!argument.known_value) {
            auto constraint = argument.constraint_expression(Location{}, renamed_name);
            if (constraint.has_value()) {
                if (constraint_expression.has_value()) {
                    constraint_expression = BinaryExpression::create(Location{}, constraint_expression.value(), BinaryExpression::Operation::LOGICAL_AND, constraint.value());
                }
                else {
                    constraint_expression = constraint;
                }
            }
        }
        else {
            scope->add(std::make_unique<Constant>(Location{}, name, Visibility::SCOPE, containing_scope, false, ValueExpression::create({}, *argument.known_value)));
        }
    }

    auto body = addressing_mode.get().encoding.clone();

    if (!argument_aliases.empty()) {
        for (const auto& [original_name, renamed_name] : argument_aliases) {
            scope->add(std::make_unique<Constant>(Location{}, original_name, Visibility::SCOPE, scope, false, VariableExpression::create(Location(), renamed_name)));
        }
    }

    if (scope) {
        body = ScopeBody::create(scope, body);
    }

    return {constraint_expression, body};
}

InstructionInvocation::Argument::Argument(const AddressingMode::Argument* definition, Node* node) : definition(definition) {
    if (node == nullptr) {
        known_value = definition->default_value;
        valid = true;
    }
    else {
        auto argument_type = definition->type;

        if (auto enum_argument_type = argument_type->as<ArgumentTypeEnum>()) {
            if (node->type() != Node::KEYWORD) {
                valid = false;
                return;
            }
            auto keyword = dynamic_cast<TokenNode*>(node)->as_symbol();
            if (!enum_argument_type->has_entry(keyword)) {
                valid = false;
                return;
            }
        }
        else if (node->type() != Node::EXPRESSION) {
            valid = false;
            return;
        }
        else {
            auto expression = dynamic_cast<ExpressionNode*>(node)->expression;
            known_value = expression.value();
            valid = argument_type->is_valid(expression);
        }
    }
}

std::optional<Expression> InstructionInvocation::Argument::constraint_expression(const Location& location, Symbol name) const {
    if (known_value.has_value()) {
        return {};
    }
    else {
        return definition->type->constraint_expression(location, name);
    }
}
