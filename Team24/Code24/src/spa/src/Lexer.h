#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace backend {
namespace lexer {
enum TokenType {
    // Characters
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    SEMICOLON,
    COMMA,
    UNDERSCORE,
    DOUBLE_QUOTE,
    SINGLE_EQ, // To prevent ambiguity
    NOT,
    ANDAND,
    OROR,
    EQEQ,
    NEQ,
    GT,
    GTE,
    LT,
    LTE,
    PLUS,
    MINUS,
    MULT,
    DIV,
    MOD,

    // Words, which have value in them.
    NAME,
    INTEGER,
};

std::string prettyPrintType(TokenType t);

struct Token {
    // Required
    TokenType type;
    int line;
    int linePosition;

    // Use only for NAME and INTEGER
    std::string nameValue;
    std::string integerValue;

    explicit Token(TokenType t) : type(t), line(), linePosition(), nameValue(), integerValue(){};
};

std::vector<Token> tokenize(std::istream& stream);
} // namespace lexer
} // namespace backend
