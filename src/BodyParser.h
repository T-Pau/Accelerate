/*
BodyParser.h --

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

#ifndef BODY_PARSER_H
#define BODY_PARSER_H

#include "Token.h"
#include "Tokenizer.h"
#include "CPU.h"
#include "ObjectFile.h"
#include "Body.h"
#include "IfBody.h"
#include "LabelBody.h"
#include "SizeRange.h"

class BodyParser {
public:
    explicit BodyParser(Tokenizer& tokenizer): end_token(Token::greater), tokenizer(tokenizer) {} // ParsedValue
    BodyParser(Tokenizer& tokenizer, const CPU* cpu): allow_memory(true), cpu(cpu), tokenizer(tokenizer) {} // Target
    BodyParser(Tokenizer& tokenizer, Object* object, const CPU* cpu, std::shared_ptr<Environment> environment): cpu(cpu), environment(std::move(environment)), object(object), tokenizer(tokenizer) {} // Assembler

    static void setup(FileTokenizer& tokenizer);

    Body parse();

private:
    class BodyIndex {
      public:
        BodyIndex(size_t if_index, size_t clause_index): if_index(if_index), clause_index(clause_index) {}

        size_t if_index;
        size_t clause_index;
    };

    bool allow_memory = false;
    const CPU* cpu = nullptr;
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    Token end_token = Token::curly_close;
    Object* object = nullptr;
    Tokenizer& tokenizer;

    uint64_t next_label = 0;
    Body body;
    std::vector<std::vector<IfBodyClause>> ifs;
    std::vector<BodyIndex> body_indices;
    Body *current_body = &body;

    [[nodiscard]] bool allow_instructions() const {return cpu != nullptr;}
    void parse_assignment(Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    std::shared_ptr<Node> parse_instruction_argument(const Token& token);
    void parse_label(Visibility visibility, const Token& name);
    void parse_unnamed_label();

    void add_constant(Visibility visibility, Token name, const Expression& value);
    [[nodiscard]] SizeRange current_size();
    Body* get_body(const BodyIndex& body_index) {return &ifs[body_index.if_index][body_index.clause_index].body;}
    [[nodiscard]] Expression get_pc(std::shared_ptr<Label> label) const;
    [[nodiscard]] std::shared_ptr<Label> get_label(bool& is_anonymous);
    [[nodiscard]] Symbol object_name() const {return object ? object->name.as_symbol() : Symbol();};
    void push_clause(Expression condition);
    void push_body(const BodyIndex& body_index);
    void pop_body();

    void handle_name(Visibility visibility, Token name);
    void parse_data();
    void parse_error();
    void parse_else();
    void parse_else_if();
    void parse_end();
    void parse_if();
    void parse_memory();

    static const Symbol symbol_pc;
    static const Token token_data;
    static const Token token_else;
    static const Token token_else_if;
    static const Token token_end;
    static const Token token_error;
    static const Token token_if;
    static const Token token_memory;

    static const std::unordered_map<Symbol, void (BodyParser::*)()> directive_parser_methods;
};

#endif // BODY_PARSER_H
