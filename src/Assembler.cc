//
// Created by Dieter Baron on 30.12.22.
//

#include <iostream>

#include "FileReader.h"
#include "Tokenizer.h"

std::vector<std::string> type_names = {
        ":",
        ",",
        "{",
        "}",
        "directive",
        "end",
        "error",
        ">",
        "#",
        "<",
        "-",
        "name",
        "newline",
        "integer",
        "(",
        ")",
        "+",
        "[",
        "]",
        "*",
        "string"
};

int main(int argc, char *argv[]) {
    auto reader = FileReader();

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " diagnostics_file\n";
    }

    auto lines = reader.read(argv[1]);

    auto tokenizer = Tokenizer();

    tokenizer.push(argv[1], lines);

    while (true) {
        auto token = tokenizer.next();

        if (token.type == Token::END) {
            break;
        }

        std::cout << token.location.file << ":" << token.location.line_number << "." << token.location.start_column << "-" << token.location.end_column
                  << "  " << type_names[token.type];
        switch (token.type) {
            case Token::COLON:
            case Token::COMMA:
            case Token::CURLY_PARENTHESIS_CLOSE:
            case Token::CURLY_PARENTHESIS_OPEN:
            case Token::END:
            case Token::EQUAL:
            case Token::GREATER:
            case Token::HASH:
            case Token::LESS:
            case Token::MINUS:
            case Token::NEWLINE:
            case Token::PARENTHESIS_CLOSE:
            case Token::PARENTHESIS_OPEN:
            case Token::PLUS:
            case Token::SQUARE_PARENTHESIS_CLOSE:
            case Token::SQUARE_PARENTHESIS_OPEN:
            case Token::STAR:
                break;

            case Token::DIRECTIVE:
            case Token::ERROR:
            case Token::NAME:
            case Token::STRING:
                std::cout << "(" << token.name << ")";
                break;

            case Token::NUMBER:
                std::cout << "(" << token.value.integer << ")";
                break;
        }

        std::cout << "\n";
    }
}
