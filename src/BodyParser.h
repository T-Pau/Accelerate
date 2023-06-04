//
// Created by Dieter Baron on 08.05.23.
//

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
