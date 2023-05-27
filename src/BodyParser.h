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
    BodyParser(Tokenizer& tokenizer, Symbol object_name, const CPU* cpu, ObjectFile* object_file, std::shared_ptr<Environment> environment): cpu(cpu), environment(std::move(environment)), object_name(object_name), object_file(object_file), tokenizer(tokenizer) {} // Assembler

    static void setup(FileTokenizer& tokenizer);

    Body parse();

private:
    bool allow_memory = false;
    const CPU* cpu = nullptr;
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    Token end_token = Token::curly_close;
    Symbol object_name;
    ObjectFile* object_file = nullptr;
    Tokenizer& tokenizer;

    uint64_t next_label = 0;
    Body body;
    std::vector<std::vector<IfBodyClause>> ifs;
    std::vector<Body*> bodies;
    Body *current_body = nullptr;

    [[nodiscard]] bool allow_instructions() const {return cpu != nullptr;}
    [[nodiscard]] bool allow_labels() const {return !object_name.empty();}
    void parse_assignment(Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    std::shared_ptr<Node> parse_instruction_argument(const Token& token);
    void parse_label(Visibility visibility, const Token& name);
    void parse_unnamed_label();

    void add_constant(Visibility visibility, Token name, const Expression& value);
    [[nodiscard]] SizeRange current_size() const;
    [[nodiscard]] Expression get_pc(std::shared_ptr<Label> label) const;
    [[nodiscard]] std::shared_ptr<Label> get_label(bool& is_anonymous);
    void push_clause(Expression condition);
    void push_body(Body* new_body);
    void pop_body();

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
