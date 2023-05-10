/*
InstructionEncoder.h --

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

#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H


#include "Target.h"
#include "IfBodyElement.h"
#include "DataBodyElement.h"
#include "InRangeExpression.h"
#include "ValueExpression.h"
#include "LabelBodyElement.h"


class InstructionEncoder {
public:
    explicit InstructionEncoder(const CPU* cpu): cpu(cpu) {}

    [[nodiscard]] std::shared_ptr<BodyElement> encode(const Token& name, const std::vector<std::shared_ptr<Node>>& arguments, const std::shared_ptr<Environment>& environment);

private:
    class Variant {
    public:
        Expression argument_constraints = Expression(Value(true));
        Expression encoding_constraints = Expression(Value(true));
        std::shared_ptr<BodyElement> data;

        void add_argument_constraint(const Expression& sub_constraint) {return add_constraint(argument_constraints, sub_constraint);}
        void add_encoding_constraint(const Expression& sub_constraint) {return add_constraint(encoding_constraints, sub_constraint);}

        explicit operator bool() const;

    private:
        static void add_constraint(Expression& constraint, const Expression& sub_constraint);
    };

    Variant encode(const Instruction* instruction, const AddressingModeMatcherResult& match, const std::vector<std::shared_ptr<Node>>& arguments, std::shared_ptr<Environment> outer_environment) const;

    const CPU* cpu;
};

#endif // INSTRUCTION_ENCODER_H
