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
#include "Util.h"

Body InstructionEncoder::encode(const Token& name, const std::vector<std::shared_ptr<Node>>& arguments, const std::shared_ptr<Environment>& environment, const SizeRange& offset) {
    const auto instruction = cpu->instruction(name.as_symbol());
    if (instruction == nullptr) {
        throw ParseException(name, "unknown instruction '%s'", name.as_string().c_str());
    }

    auto matches = cpu->match_addressing_modes(arguments);
    if (matches.empty()) {
        throw ParseException(name, "addressing mode not recognized");
    }

    auto variants = std::vector<Variant>();

    auto supported = false;

    for (const auto &match: matches) {
        if (instruction->has_addressing_mode(match.addressing_mode)) {
            supported = true;
            auto variant = encode(instruction, match, arguments, environment, offset);
            auto constraint =  variant.argument_constraints.value() && variant.encoding_constraints.value();

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
        if (supported) {
            throw ParseException(name, "arguments out of range");
        }
        else {
            if (matches.size() == 1) {
                throw ParseException(name, "instruction %s doesn't support addressing mode %s", name.as_string().c_str(), matches.begin()->addressing_mode.c_str());
            } else {
                auto modes = std::vector<Symbol>();
                for (const auto &match: matches) {
                    modes.emplace_back(match.addressing_mode);
                }
                std::sort(modes.begin(), modes.end());
                throw ParseException(name, "instruction %s doesn't support any of the addressing modes %s", name.as_string().c_str(), join(modes).c_str());
            }
        }
    }
    else if (variants.size() == 1) {
        auto constraints = variants.front().argument_constraints;
        if (constraints.has_value() && constraints.value()->boolean_value()) {
            return variants.front().data;
        }
    }

    auto clauses = std::vector<IfBodyClause>();

    for (const auto &variant: variants) {
        auto constraints = variant.argument_constraints;
        if (&variant != &variants.back()) {
            constraints = Expression(constraints, Expression::BinaryOperation::LOGICAL_AND, variant.encoding_constraints);
        }
        clauses.emplace_back(constraints, variant.data);
    }

    clauses.emplace_back(Expression(Value(true)), Body(name.location, "arguments out of range"));

    return Body(clauses);
}

InstructionEncoder::Variant InstructionEncoder::encode(const Instruction* instruction, const AddressingModeMatcherResult& match, const std::vector<std::shared_ptr<Node>>& arguments, std::shared_ptr<Environment> outer_environment, const SizeRange& offset) const {
    auto environment = std::make_shared<Environment>(std::move(outer_environment));

    const auto addressing_mode = cpu->addressing_mode(match.addressing_mode);
    const auto& notation = addressing_mode->notations[match.notation_index];
    auto it_notation = notation.elements.begin();
    auto it_arguments = arguments.begin();

    Variant variant;

    while (it_notation != notation.elements.end()) {
        if (it_notation->is_argument()) {
            auto argument_type = addressing_mode->argument(it_notation->symbol);
            switch (argument_type->type()) {
                case ArgumentType::ANY:
                case ArgumentType::ENCODING: {
                    if ((*it_arguments)->type() != Node::EXPRESSION) {
                        throw ParseException((*it_arguments)->get_location(), "any argument is not an expression");
                    }
                    auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;

                    environment->add(it_notation->symbol, expression);
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
                    environment->add(it_notation->symbol, Expression(enum_type->entry(value_name)));
                    break;
                }

                case ArgumentType::MAP: {
                    if ((*it_arguments)->type() != Node::EXPRESSION) {
                        throw ParseException((*it_arguments)->get_location(), "map argument is not an expression");
                    }
                    auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;
                    if (!expression.has_value()) {
                        throw ParseException((*it_arguments)->get_location(), "map argument is not an integer");
                    }
                    auto map_type = dynamic_cast<const ArgumentTypeMap*>(argument_type);
                    auto value = expression.value().value();
                    if (!map_type->has_entry(value)) {
                        throw ParseException((*it_arguments)->get_location(), "invalid map argument");
                    }
                    environment->add(it_notation->symbol, Expression(map_type->entry(value)));
                    break;
                }

                case ArgumentType::RANGE:
                    auto range_type = dynamic_cast<const ArgumentTypeRange*>(argument_type);

                    if ((*it_arguments)->type() != Node::EXPRESSION) {
                        throw ParseException((*it_arguments)->get_location(), "range argument is not an expression");
                    }
                    auto expression = std::dynamic_pointer_cast<ExpressionNode>(*it_arguments)->expression;

                    variant.add_argument_constraint(InRangeExpression::create(Expression(range_type->lower_bound), Expression(range_type->upper_bound), expression));
                    environment->add(it_notation->symbol, expression);
                    break;
            }
        }

        it_notation++;
        it_arguments++;
    }

    environment->add(Assembler::symbol_opcode, Expression(instruction->opcode(match.addressing_mode)));

    variant.data = addressing_mode->encoding;
    variant.data.evaluate(Symbol(), environment, true, offset);

    auto data = variant.data.as_data();
    for (const auto& datum: data->data) {
        if (datum.encoding.has_value()) {
            auto lower = datum.encoding->minimum_value();
            auto upper = datum.encoding->maximum_value();
            if (lower.has_value() && upper.has_value()) {
                variant.add_encoding_constraint(InRangeExpression::create(Expression(*lower), Expression(*upper), datum.expression));
            }
        }
    }

    return variant;
}

void InstructionEncoder::Variant::add_constraint(Expression& constraints, const Expression& sub_constraint) {
    constraints = Expression(constraints, Expression::BinaryOperation::LOGICAL_AND, sub_constraint);
}

InstructionEncoder::Variant::operator bool() const {
    auto value = argument_constraints.value() && encoding_constraints.value();
    return value.has_value() && value->boolean_value();
}

