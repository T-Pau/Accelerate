#ifdef IN_XLR8_BODY_PARSER_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_BODY_PARSER_H
#ifndef HAD_XLR8_BODY_PARSER_H
#define HAD_XLR8_BODY_PARSER_H

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

#include "Body/Body.h"
#include "Body/IfBody.h"
#include "CPU.h"
#include "Expression/ValueExpression.h"
#include "Scope.h"
#include "SizeRange.h"
#include "Token.h"
#include "Tokenizer.h"
#include "Visibility.h"

/**
 * @brief This class parses object and macro bodies.
 */
class BodyParser {
  public:
    /**
     * @brief Type of body being parsed.
     */
    enum ParsingType {
        ENTITY,          ///< Parsing a macro or object from source.
        OUTPUT,          ///< Parsing `.output` section from target.
        STRUCTURED_VALUE ///< Parsing body in StructuredValue from object file.
    };

    /**
     * @brief Constructs a BodyParser for parsing a StructuredValue.
     *
     * @param tokenizer The tokenizer to parse the StructuredValue from.
     * @param environment The environment the body is in.
     */
    explicit BodyParser(Tokenizer& tokenizer, std::shared_ptr<Scope> environment) : parsing_type(STRUCTURED_VALUE), end_token(Token::greater), tokenizer(tokenizer), environment(environment) {}

    /**
     * @brief Constructs a BodyParser for parsing an entity or output.
     *
     * If `allow_assigns` is `true`, it will parse an entity (macro or object) and allow assignments. If `false`, it will parse an output section.
     *
     * @param tokenizer The tokenizer to parse the entity or output from.
     * @param cpu The CPU context.
     * @param environment The environment the body is in.
     * @param allow_assigns Whether assignments are allowed.
     */
    BodyParser(Tokenizer& tokenizer, const CPU* cpu, std::shared_ptr<Scope> environment, bool allow_assigns) : parsing_type(allow_assigns ? ENTITY : OUTPUT), cpu(cpu), tokenizer(tokenizer), environment(environment) {}

    /**
     * @brief Sets up the FileTokenizer for parsing bodies.
     *
     * @param tokenizer The FileTokenizer to set up.
     */
    static void setup(FileTokenizer& tokenizer);

    /**
     * @brief Parses the body from the tokenizer.
     *
     * @return The parsed body.
     */
    Body parse();

  private:
    class IfNesting;
    class RepeatNesting;
    class ScopeNesting;

    class Nesting {
      public:
        virtual ~Nesting() = default;

        virtual Body* operator[](size_t index) = 0;
        virtual Body body() = 0;

        IfNesting* as_if() { return dynamic_cast<IfNesting*>(this); }

        RepeatNesting* as_repeat() { return dynamic_cast<RepeatNesting*>(this); }

        ScopeNesting* as_scope() { return dynamic_cast<ScopeNesting*>(this); }

        bool is_if() { return as_if(); }

        bool is_repeat() { return as_repeat(); }

        bool is_scope() { return as_scope(); }
    };

    class IfNesting : public Nesting {
      public:
        Body* operator[](size_t index) override { return &clauses[index].body; }

        Body body() override { return IfBody::create(clauses); }

        void add_clause(Expression condition) { clauses.emplace_back(IfBodyClause(std::move(condition), {})); }

        [[nodiscard]] size_t size() const { return clauses.size(); }

      private:
        std::vector<IfBodyClause> clauses;
    };

    class RepeatNesting : public Nesting {
      public:
        RepeatNesting(Symbol variable, std::optional<Expression> start, Expression end) : variable{variable}, start{start.value_or(ValueExpression::create({}, Value(uint64_t{0})))}, end{std::move(end)} {}

        Body* operator[](size_t index) override { return &inner_body; }

        Body body() override;

      private:
        Symbol variable;
        Expression start;
        Expression end;
        Body inner_body;
    };

    class ScopeNesting : public Nesting {
      public:
        Body* operator[](size_t index) override { return &inner_body; }

        Body body() override { /* TODO: implement */ /* return inner_body; */ return Body{}; }

        Body inner_body;
    };

    class NestingIndex {
      public:
        NestingIndex(size_t nesting_index, size_t sub_index) : nesting_index(nesting_index), sub_index(sub_index) {}

        size_t nesting_index;
        size_t sub_index;
    };

    ParsingType parsing_type;
    const CPU* cpu = nullptr;
    std::shared_ptr<Scope> environment;
    Token end_token = Token::curly_close;
    Tokenizer& tokenizer;

    uint64_t next_label = 0;
    Body body;
    std::vector<std::unique_ptr<Nesting>> nesting;
    std::vector<NestingIndex> nesting_indices;
    Body* current_body = &body;

    [[nodiscard]] bool allow_memory() const { return parsing_type == OUTPUT; }

    [[nodiscard]] bool allow_instructions() const { return cpu != nullptr; }

    [[nodiscard]] bool allow_assignment() const { return parsing_type == ENTITY; }

    [[nodiscard]] bool is_defined(Symbol symbol) const { return environment->is_defined(symbol); }

    [[nodiscard]] bool is_defined(const Token& token) const { return is_defined(token.as_symbol()); }

    void parse_assignment(Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    std::shared_ptr<Node> parse_instruction_argument(const Token& token);
    void parse_label(Visibility visibility, const Token& name);
    void parse_unnamed_label();

    void add_constant(Visibility visibility, const Token& name, const Expression& value);
    [[nodiscard]] SizeRange current_size();

    [[nodiscard]] Body* get_body(const NestingIndex& nesting_index) const { return (*nesting[nesting_index.nesting_index])[nesting_index.sub_index]; }

    [[nodiscard]] Expression get_pc_label();
    [[nodiscard]] Expression get_label(const Location& location, Symbol name) const;
    void push_clause(Expression condition);
    void push_body(const NestingIndex& body_index);
    void pop_body();

    void handle_name(Visibility visibility, const Token& name);
    void parse_binary_file();
    void parse_checksum();
    void parse_data();
    void parse_error();
    void parse_else();
    void parse_else_if();
    void parse_if();
    void parse_memory();
    void parse_repeat();
    void parse_scope();

    static const Symbol symbol_pc;
    static const Token token_binary_file;
    static const Token token_checksum;
    static const Token token_data;
    static const Token token_end;
    static const Token token_else;
    static const Token token_else_if;
    static const Token token_error;
    static const Token token_if;
    static const Token token_length;
    static const Token token_memory;
    static const Token token_repeat;
    static const Token token_scope;
    static const Token token_start;

    static const std::unordered_map<Symbol, void (BodyParser::*)()> directive_parser_methods;
};

#endif // HAD_XLR8_BODY_PARSER_H
#undef IN_XLR8_BODY_PARSER_H
