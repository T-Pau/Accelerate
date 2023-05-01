/*
InstructionEncoder.cc --

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

#include "InstructionEncoder.h"

#include <utility>

#include "Assembler.h"
#include "ParseException.h"
#include "TokenNode.h"
#include "ValueExpression.h"
#include "ExpressionNode.h"
#include "BinaryExpression.h"

std::shared_ptr<BodyElement>
InstructionEncoder::encode(const Token& name, const std::vector<std::shared_ptr<Node>>& arguments, const std::shared_ptr<Environment>& environment) {
    const auto instruction = target.cpu->instruction(name.as_symbol());
    if (instruction == nullptr) {
        throw ParseException(name, "unknown instruction '%s'", name.as_string().c_str());
    }

    auto matches = target.cpu->match_addressing_modes(arguments);
    if (matches.empty()) {
        throw ParseException(name, "addressing mode not recognized");
    }

    auto variants = std::vector<Variant>();

    for (const auto &match: matches) {
        if (instruction->has_addressing_mode(match.addressing_mode)) {
            auto variant = encode(instruction, match, arguments, environment);
            auto constraint =  variant.argument_constraints->value() && variant.encoding_constraints->value();

            if (constraint.has_value() && !*constraint) {
                continue;
            }
            variants.emplace_back(std::move(variant));
            if (constraint.has_value() && *constraint) {
                break;
            }
        }
    }

    if (variants.empty()) {
        // TODO: store best matching error
        throw ParseException(name, "can't encode");
    }
    else if (variants.size() == 1) {
        auto constraints = variants.front().argument_constraints;
        if (constraints->has_value() && constraints->value()->boolean_value()) {
            return variants.front().data;
        }
    }

    auto body = std::make_shared<IfBodyElement>();

    for (const auto &variant: variants) {
        auto constraints = variant.argument_constraints;
        if (&variant != &variants.back()) {
            constraints = BinaryExpression::create(constraints, BinaryExpression::LOGICAL_AND, variant.encoding_constraints);
        }
        body->append(constraints, variant.data);
    }

    // TODO: add error clause

    return body;
}

InstructionEncoder::Variant InstructionEncoder::encode(const Instruction* instruction, const AddressingModeMatcherResult& match, const std::vector<std::shared_ptr<Node>>& arguments, std::shared_ptr<Environment> outer_environment) const {
    auto environment = Environment(std::move(outer_environment));

    const auto addressing_mode = target.cpu->addressing_mode(match.addressing_mode);
    const auto& notation = addressing_mode->notations[match.notation_index];
    auto it_notation = notation.elements.begin();
    auto it_arguments = arguments.begin();

    Variant variant;

    while (it_notation != notation.elements.end()) {
        if (it_notation->is_argument()) {
            auto argument_type = addressing_mode->argument(it_notation->symbol);
            switch (argument_type->type()) {
                case ArgumentType::ANY: {
                    if ((*it_arguments)->type() != Node::EXPRESSION) {
                        throw ParseException((*it_arguments)->get_location(), "any argument is not an expression");
                    }
                    auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;

                    environment.add(it_notation->symbol, expression);
                    break;
                }

                case ArgumentType::ENUM: {
                    if ((*it_arguments)->type() != Node::KEYWORD) {
                        throw ParseException((*it_arguments)->get_location(), "enum argument is not a keyword");
                    }
                    auto enum_type = dynamic_cast<const ArgumentTypeEnum *>(argument_type);
                    auto value_name = std::dynamic_pointer_cast<TokenNode>(*it_arguments)->as_symbol();
                    if (!enum_type->has_entry(value_name)) {
                        throw ParseException((*it_arguments)->get_location(), "invalid enum argument");
                    }
                    environment.add(it_notation->symbol, std::make_shared<ValueExpression>(enum_type->entry(value_name)));
                    break;
                }

                case ArgumentType::MAP: {
                    if ((*it_arguments)->type() != Node::EXPRESSION) {
                        throw ParseException((*it_arguments)->get_location(), "map argument is not an expression");
                    }
                    auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;
                    if (!expression->has_value()) {
                        throw ParseException((*it_arguments)->get_location(), "map argument is not an integer");
                    }
                    auto map_type = dynamic_cast<const ArgumentTypeMap*>(argument_type);
                    auto value = expression->value().value();
                    if (!map_type->has_entry(value)) {
                        throw ParseException((*it_arguments)->get_location(), "invalid map argument");
                    }
                    environment.add(it_notation->symbol, std::make_shared<ValueExpression>(map_type->entry(value)));
                    break;
                }

                case ArgumentType::RANGE:
                    auto range_type = dynamic_cast<const ArgumentTypeRange*>(argument_type);

                    if ((*it_arguments)->type() != Node::EXPRESSION) {
                        throw ParseException((*it_arguments)->get_location(), "range argument is not an expression");
                    }
                    auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;

                    variant.add_argument_constraint(InRangeExpression::create(std::make_shared<ValueExpression>(range_type->lower_bound), std::make_shared<ValueExpression>(range_type->upper_bound), expression));
                    environment.add(it_notation->symbol, expression);
                    break;
            }
        }

        it_notation++;
        it_arguments++;
    }

    environment.add(Assembler::symbol_opcode, std::make_shared<ValueExpression>(instruction->opcode(match.addressing_mode)));
    //TODO: environment.add(symbol_pc, get_pc());

    variant.data = BodyElement::evaluate(addressing_mode->encoding, environment);

    auto data = std::dynamic_pointer_cast<DataBodyElement>(variant.data);
    for (const auto& datum: data->data) {
        if (datum.encoding.has_value()) {
            auto lower = datum.encoding->minimum_value();
            auto upper = datum.encoding->maximum_value();
            if (lower.has_value() && upper.has_value()) {
                variant.add_encoding_constraint(InRangeExpression::create(std::make_shared<ValueExpression>(*lower), std::make_shared<ValueExpression>(*upper), datum.expression));
            }
        }
    }

    return variant;
}

void InstructionEncoder::Variant::add_constraint(std::shared_ptr<Expression>& constraints, const std::shared_ptr<Expression>& sub_constraint) {
    constraints = BinaryExpression::create(constraints, BinaryExpression::LOGICAL_AND, sub_constraint);
}

InstructionEncoder::Variant::operator bool() const {
    auto value = argument_constraints->value() && encoding_constraints->value();
    return value.has_value() && value->boolean_value();
}

