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
#include "IfBodyElement.h"
#include "LabelBodyElement.h"
#include "SizeRange.h"

class BodyParser {
public:
    explicit BodyParser(Tokenizer& tokenizer): end_token(Token::greater), tokenizer(tokenizer) {} // ParsedValue
    BodyParser(Tokenizer& tokenizer, Symbol object_name, const CPU* cpu): allow_memory(true), cpu(cpu), object_name(object_name), tokenizer(tokenizer) {} // Target
    BodyParser(Tokenizer& tokenizer, Symbol object_name, const CPU* cpu, ObjectFile* object_file, std::shared_ptr<Environment> environment): cpu(cpu), environment(std::move(environment)), object_name(object_name), object_file(object_file), tokenizer(tokenizer) {} // Assembler

    static void setup(FileTokenizer& tokenizer);

    Body parse();

private:
    bool allow_memory = false;
    const CPU* cpu = nullptr;
    std::shared_ptr<Environment> environment;
    Token end_token = Token::curly_close;
    Symbol object_name;
    ObjectFile* object_file = nullptr;
    Tokenizer& tokenizer;

    uint64_t next_label = 0;
    Body body;
    std::vector<std::vector<IfBodyElement::Clause>> ifs;
    std::vector<Body*> bodies;
    Body *current_body = nullptr;

    [[nodiscard]] bool allow_instructions() const {return cpu != nullptr;}
    void parse_assignment(Object::Visibility visibility, const Token& name);
    void parse_directive(const Token& directive);
    void parse_instruction(const Token& name);
    std::shared_ptr<Node> parse_instruction_argument(const Token& token);
    void parse_label(Object::Visibility visibility, const Token& name);

    void add_constant(Object::Visibility visibility, Token name, const std::shared_ptr<Expression>& value);
    [[nodiscard]] SizeRange current_size() const;
    [[nodiscard]] std::shared_ptr<Expression> get_pc(const std::shared_ptr<LabelBodyElement>& label) const;
    [[nodiscard]] std::shared_ptr<LabelBodyElement> get_label(bool& is_anonymous);
    void push_clause(std::shared_ptr<Expression> condition);
    void push_body(Body* new_body);
    void pop_body();

    void parse_data();
    void parse_else();
    void parse_else_if();
    void parse_end();
    void parse_if();
    void parse_memory();

    static const Symbol symbol_pc;
    static const Token token_data;
    static const Token token_end;
    static const Token token_else;
    static const Token token_else_if;
    static const Token token_if;
    static const Token token_memory;

    static const std::unordered_map<Symbol, void (BodyParser::*)()> directive_parser_methods;
};

#endif // BODY_PARSER_H
