//
// Created by Dieter Baron on 08.05.23.
//

#include "BodyParser.h"

#include <utility>

#include "ParseException.h"
#include "FileReader.h"
#include "ExpressionParser.h"
#include "ValueExpression.h"
#include "ExpressionNode.h"
#include "InstructionEncoder.h"
#include "LabelExpression.h"
#include "TokenNode.h"
#include "MemoryBody.h"

const Symbol BodyParser::symbol_pc = Symbol(".pc");
const Token BodyParser::token_data = Token(Token::DIRECTIVE, "data");
const Token BodyParser::token_else = Token(Token::DIRECTIVE, "else");
const Token BodyParser::token_else_if = Token(Token::DIRECTIVE, "else_if");
const Token BodyParser::token_end = Token(Token::DIRECTIVE, "end");
const Token BodyParser::token_error = Token(Token::DIRECTIVE, "error");
const Token BodyParser::token_if = Token(Token::DIRECTIVE, "if");
const Token BodyParser::token_memory = Token(Token::DIRECTIVE, "memory");

const std::unordered_map<Symbol, void (BodyParser::*)()> BodyParser::directive_parser_methods = {
        {token_data.as_symbol(), &BodyParser::parse_data},
        {token_else.as_symbol(), &BodyParser::parse_else},
        {token_else_if.as_symbol(), &BodyParser::parse_else_if},
        {token_end.as_symbol(), &BodyParser::parse_end},
        {token_error.as_symbol(), &BodyParser::parse_error},
        {token_if.as_symbol(), &BodyParser::parse_if},
        {token_memory.as_symbol(), &BodyParser::parse_memory}
};

void BodyParser::setup(FileTokenizer &tokenizer) {
    // TODO: implement
}

Body BodyParser::parse() {
    body = Body();
    push_body(&body);

    while (!tokenizer.ended()) {
        try {
            auto token = tokenizer.next();

            switch (token.get_type()) {
                case Token::END:
                    throw ParseException(token, "unclosed symbol body");

                case Token::DIRECTIVE:
                    parse_directive(token);
                    break;

                case Token::NAME: {
                    auto token2 = tokenizer.next();
                    if (token2 == Token::colon) {
                        parse_label(Object::OBJECT, token);
                    }
                    else if (token2 == Token::equals) {
                        parse_assignment(Object::OBJECT, token);
                    }
                    else {
                        tokenizer.unget(token2);
                        throw ParseException(token, "unexpected name");
                    }
                    break;
                }

                case Token::INSTRUCTION:
                    if (allow_instructions()) {
                        parse_instruction(token);
                    }
                    else {
                        throw ParseException(token, "unexpected %s", token.type_name());
                    }
                    break;

                case Token::NEWLINE:
                    // ignore
                    break;

                case Token::PUNCTUATION:
                    if (token == end_token) {
                        // TODO: check that ifs is empty
                        body.evaluate(*environment);
                        return body;
                    }
                    else {
                        if (allow_instructions() && cpu->uses_empty_mnemonic()) {
                            tokenizer.unget(token);
                            parse_instruction(Token(Token::NAME, token.location, Symbol()));
                        }
                        else {
                            throw ParseException(token, "unexpected %s", token.type_name());
                        }
                    }
                    break;

                default:
                    if (allow_instructions() && cpu->uses_empty_mnemonic()) {
                        tokenizer.unget(token);
                        parse_instruction(Token(Token::NAME, token.location, Symbol()));
                    }
                    else {
                        throw ParseException(token, "unexpected %s", token.type_name());
                    }
                    break;
            }
        }
        catch (ParseException& ex) {
            FileReader::global.error(ex.location, "%s", ex.what());
        }
    }

    throw ParseException(Location(), "unclosed body"); // TODO: location
}

void BodyParser::add_constant(Object::Visibility visibility, Token name, const Expression& value) {
    if (visibility != Object::OBJECT) {
        if (!object_file) {
            throw ParseException(name, "unsupported visibility");
        }
        object_file->add_constant(name.as_symbol(), visibility, value);
    }
    environment->add(name.as_symbol(), value);
}


std::shared_ptr<Label> BodyParser::get_label(bool& is_anonymous) {
    auto trailing_label = current_body->back();
    if (trailing_label.has_value() && trailing_label->is_label()) {
        is_anonymous = false;
        return trailing_label->as_label()->label;
    }
    next_label += 1;
    auto name = Symbol(".label_" + std::to_string(next_label));
    is_anonymous = true;
    return std::make_shared<Label>(name, current_size());
}


Expression BodyParser::get_pc(std::shared_ptr<Label> label) const {
    return {Expression(object_name), Expression::BinaryOperation::ADD, Expression(std::make_shared<LabelExpression>(std::move(label)))};
}


void BodyParser::parse_directive(const Token& directive) {
    auto it = directive_parser_methods.find(directive.as_symbol());
    if (it == directive_parser_methods.end() || (directive == token_memory && !allow_memory)) {
        throw ParseException(directive, "unknown directive");
    }
    (this->*it->second)();
}


void BodyParser::parse_data() {
    current_body->append(ExpressionParser(tokenizer).parse_list());
}

void BodyParser::parse_else() {
    if (ifs.empty()) {
        throw Exception(".else outside .if");
    }
    push_clause(Expression(Value(true)));
}

void BodyParser::parse_else_if() {
    if (ifs.empty()) {
        throw Exception(".else_if outside .if");
    }
    push_clause(ExpressionParser(tokenizer).parse());
}

void BodyParser::parse_end() {
    if (ifs.empty()) {
        throw Exception(".end outside .if");
    }
    pop_body();
    current_body->append(Body(ifs.back()));
    ifs.pop_back();
}

void BodyParser::parse_if() {
    ifs.emplace_back();
    push_clause(ExpressionParser(tokenizer).parse());
}

std::shared_ptr<Node> BodyParser::parse_instruction_argument(const Token& token) {
    switch (token.get_type()) {
        case Token::PUNCTUATION:
            if (cpu->uses_punctuation(token.as_symbol())) {
                return std::make_shared<TokenNode>(token);
            }
            break;

        case Token::KEYWORD:
            // TODO: check it's used by target.cpu, throw otherwise
            return std::make_shared<TokenNode>(token);

        case Token::DIRECTIVE:
        case Token::INSTRUCTION:
            throw ParseException(token, "unexpected %s", token.type_name());

        default:
            break;
    }

    tokenizer.unget(token);
    return std::make_shared<ExpressionNode>(ExpressionParser(tokenizer).parse());
}


void BodyParser::parse_label(Object::Visibility visibility, const Token& name) {
    auto label = std::make_shared<Label>(name.as_symbol(), SizeRange(current_body->size_range()));
    current_body->append(Body(label));
    add_constant(visibility, name, get_pc(label));
    environment->add(Symbol(".label_offset(" + name.as_string() + ")"), Expression(std::make_shared<LabelExpression>(label)));
}


void BodyParser::parse_memory() {
    auto expression_parser = ExpressionParser(tokenizer);
    auto bank = Expression(uint64_t(0));
    auto start_address = expression_parser.parse();
    tokenizer.expect(Token::comma);
    auto end_address = expression_parser.parse();
    auto token = tokenizer.next();
    if (token == Token::comma) {
        bank = start_address;
        start_address = end_address;
        end_address = expression_parser.parse();
    }
    else {
        tokenizer.unget(token);
    }
    current_body->append(Body(bank, start_address, end_address));
}

void BodyParser::parse_assignment(Object::Visibility visibility, const Token &name) {
    // TODO: implement
}

void BodyParser::parse_instruction(const Token &name) {
    std::vector<std::shared_ptr<Node>> arguments;

    Token token;
    while ((token = tokenizer.next()) && !token.is_newline()) {
        if (cpu->uses_braces() && token == Token::paren_open) {
            auto node = parse_instruction_argument(tokenizer.next());
            if (node->type() != Node::EXPRESSION) {
                // place '(' non-expression
                arguments.emplace_back(parse_instruction_argument(token));
                arguments.emplace_back(node);
            }
            else {
                auto token2 = tokenizer.next();
                if (token2.is_newline()) {
                    arguments.emplace_back(node);
                    // place trailing '(' expression
                    arguments.emplace_back(parse_instruction_argument(token));
                    arguments.emplace_back(node);
                    break;
                }
                else if (token2 == Token::paren_close) {
                    auto token3 = tokenizer.next();

                    if (token3.is_newline()) {
                        // place trailing '(' expression ')'
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));
                        break;
                    }

                    tokenizer.unget(token3);

                    if (!token3.is_punctuation() || cpu->uses_punctuation(token3.as_symbol())) {
                        // place '(' expression ')' that is not part of larger expression
                        arguments.emplace_back(parse_instruction_argument(token));
                        arguments.emplace_back(node);
                        arguments.emplace_back(parse_instruction_argument(token2));

                    }
                    else {
                        // '(' expression ')' is part of larger expression
                        arguments.emplace_back(std::make_shared<ExpressionNode>(ExpressionParser(tokenizer).parse(std::dynamic_pointer_cast<ExpressionNode>(node)->expression)));
                    }
                }
                else {
                    tokenizer.unget(token2);
                    // place '(' expression that is not followed by ')'
                    arguments.emplace_back(parse_instruction_argument(Token::paren_open));
                    arguments.emplace_back(node);
                }
            }
        }
        else {
            arguments.emplace_back(parse_instruction_argument(token));
        }
    }

    auto encoder = InstructionEncoder(cpu);

    auto is_anonymous_label = false;
    auto label = get_label(is_anonymous_label);
    auto instruction = Body();
    {
        auto instruction_environment = std::make_shared<Environment>(environment);
        instruction_environment->add(symbol_pc, get_pc(label));
        instruction_environment->add(Symbol(".label_offset(" + label->name.str() + ")"), Expression(std::make_shared<LabelExpression>(label)));
        instruction = encoder.encode(name, arguments, instruction_environment, current_size());
    }
    if (is_anonymous_label) {
        if (label.use_count() > 1) {
            current_body->append(Body(label));
        }
        else {
            next_label -= 1;
        }
    }

    current_body->append(instruction);
}


void BodyParser::push_body(Body *new_body) {
    bodies.emplace_back(new_body);
    current_body = new_body;
}

void BodyParser::pop_body() {
    bodies.pop_back();
    if (bodies.empty()) {
        push_body(&body);
        throw Exception("internal error: closing outermost body");
    }
    current_body = bodies.back();
}

void BodyParser::push_clause(Expression condition) {
    ifs.back().emplace_back(IfBodyClause(std::move(condition), {}));
    push_body(&ifs.back().back().body);
}

SizeRange BodyParser::current_size() const {
    auto size = SizeRange();

    for (auto& b: bodies) {
        size += b->size_range();
    }

    return size;
}

void BodyParser::parse_error() {
    auto message = tokenizer.expect(Token::STRING, TokenGroup::newline);
    auto token = tokenizer.next();
    Location location = message.location;
    if (token == Token::paren_open) {
        location = Location(tokenizer.expect(Token::STRING, TokenGroup::newline).as_string());
        token = tokenizer.next();
        if (token == Token::comma) {
            location.start_line_number = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
            token = tokenizer.next();
            if (token == Token::comma) {
                location.start_column = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
                token = tokenizer.next();
                if (token == Token::comma) {
                    location.end_column = tokenizer.expect(Token::VALUE, TokenGroup::newline).as_unsigned();
                    token = tokenizer.next();
                }
            }
        }
        if (token != Token::paren_close) {
            throw ParseException(token, "expected ')'");
        }
    }
    else {
        tokenizer.unget(token);
    }
}
