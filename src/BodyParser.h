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
    enum ParsingType {
        ENTITY,         // Parsing a macro or object from source.
        OUTPUT,         // Parsing output section from target.
        PARSED_VALUE    // Parsing body in parsed value from object file.
    };

    // PARSED_VALUE
    explicit BodyParser(Tokenizer& tokenizer): parsing_type(PARSED_VALUE), end_token(Token::greater), tokenizer(tokenizer) {}
    // OUTPUT
    BodyParser(Tokenizer& tokenizer, const CPU* cpu): parsing_type(OUTPUT), cpu(cpu), tokenizer(tokenizer) {}
    // ENTITY
    BodyParser(Tokenizer& tokenizer, Entity* entity, const CPU* cpu): parsing_type(ENTITY), cpu(cpu), entity(entity), environment(entity->environment), tokenizer(tokenizer) {}

    static void setup(FileTokenizer& tokenizer);

    Body parse();

private:
    class IfNesting;
    class ScopeNesting;

    class Nesting {
      public:
        virtual ~Nesting() = default;

        virtual Body* operator[](size_t index) = 0;
        virtual Body body() = 0;

        IfNesting* as_if() {return dynamic_cast<IfNesting*>(this);}
        ScopeNesting* as_scope() {return dynamic_cast<ScopeNesting*>(this);}
        bool is_if() {return as_if();}
        bool is_scope() {return as_scope();}
    };

    class IfNesting: public Nesting {
      public:
        ~IfNesting() override = default;

        Body* operator[](size_t index) override {return &clauses[index].body;}
        Body body() override {return Body(clauses);}

        void add_clause(Expression condition) {clauses.emplace_back(IfBodyClause(std::move(condition), {}));}
        [[nodiscard]] size_t size() const {return clauses.size();}

      private:
        std::vector<IfBodyClause> clauses;
    };

    class ScopeNesting: public Nesting {
      public:
        ~ScopeNesting() override = default;

        Body* operator[](size_t index) override {return &inner_body;}
        Body body() override {return inner_body.scoped();}

        Body inner_body;
    };

    class NestingIndex {
      public:
        NestingIndex(size_t nesting_index, size_t sub_index): nesting_index(nesting_index), sub_index(sub_index) {}

        size_t nesting_index;
        size_t sub_index;
    };

    ParsingType parsing_type;
    const CPU* cpu = nullptr;
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    Token end_token = Token::curly_close;
    Entity* entity = nullptr;
    Tokenizer& tokenizer;

    uint64_t next_label = 0;
    Body body;
    std::vector<std::unique_ptr<Nesting>> nesting;
    std::vector<NestingIndex> nesting_indices;
    Body *current_body = &body;

    [[nodiscard]] bool allow_memory() const {return parsing_type == OUTPUT;}
    [[nodiscard]] bool allow_instructions() const {return cpu != nullptr;}
    void parse_assignment(Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    std::shared_ptr<Node> parse_instruction_argument(const Token& token);
    void parse_label(Visibility visibility, const Token& name);
    void parse_unnamed_label();

    void add_constant(Visibility visibility, Token name, const Expression& value);
    [[nodiscard]] SizeRange current_size();
    Body* get_body(const NestingIndex& nesting_index) {return (*nesting[nesting_index.nesting_index])[nesting_index.sub_index];}
    [[nodiscard]] Expression get_pc(Symbol label) const;
    [[nodiscard]] Symbol get_label(bool& is_anonymous);
    [[nodiscard]] Symbol entity_name() const {return entity ? entity->name.as_symbol() : Symbol();};
    void push_clause(Expression condition);
    void push_body(const NestingIndex& body_index);
    void pop_body();

    void handle_name(Visibility visibility, Token name);
    void parse_data();
    void parse_error();
    void parse_else();
    void parse_else_if();
    void parse_end();
    void parse_if();
    void parse_memory();
    void parse_scope();

    static const Symbol symbol_pc;
    static const Token token_data;
    static const Token token_else;
    static const Token token_else_if;
    static const Token token_end;
    static const Token token_error;
    static const Token token_if;
    static const Token token_memory;
    static const Token token_scope;

    static const std::unordered_map<Symbol, void (BodyParser::*)()> directive_parser_methods;
};

#endif // BODY_PARSER_H
