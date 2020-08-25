//
// Created by Vignesh Shankar on 25/8/20.
//

#include "Lexer.h"

namespace lexer {

// for debugging
std::map<TokenType, std::string> showtype = {
    { LBRACE, "LBRACE" },
    { RBRACE, "RBRACE" },
    { LPAREN, "LPAREN" },
    { RPAREN, "RPAREN" },
    { SEMICOLON, "SEMICOLON" },
    { SINGLE_EQ, "SINGLE_EQ" },
    { NOT, "NOT" },
    { ANDAND, "ANDAND" },
    { OROR, "OROR" },
    { EQEQ, "EQEQ" },
    { NEQ, "NEQ" },
    { GT, "GT" },
    { GTE, "GTE" },
    { LT, "LT" },
    { LTE, "LTE" },
    { PLUS, "PLUS" },
    { MINUS, "MINUS" },
    { MULT, "MULT" },
    { DIV, "DIV" },
    { MOD, "MOD" },
    { PROCEDURE, "PROCEDURE" },
    { READ, "READ" },
    { PRINT, "PRINT" },
    { WHILE, "WHILE" },
    { CALL, "CALL" },
    { IF, "IF" },
    { THEN, "THEN" },
    { ELSE, "ELSE" },
    { NAME, "NAME" },
    { INTEGER, "INTEGER" },
};

std::vector<std::pair<TokenType, std::string>> rules = {
    { LBRACE, "^\\{" },        { RBRACE, "^\\}" },
    { LPAREN, "^\\(" },        { RPAREN, "^\\)" },
    { SEMICOLON, "^;" },


    { NEQ, "^(!=)" },          { NOT, "^(!)" },

    { EQEQ, "^(==)" },         { SINGLE_EQ, "^(=)" },

    { ANDAND, "^(&&)" },       { OROR, "^(\\|\\|)" },
    { GTE, "^(>=)" },          { GT, "^(>)" },

    { LTE, "^(<=)" },          { LT, "^(<)" },

    { PLUS, "^(\\+)" },        { MINUS, "^(-)" },
    { MULT, "^(\\*)" },        { DIV, "^(\\/)" },
    { MOD, "^(%)" },           { PROCEDURE, "^(procedure)" },
    { READ, "^(read)" },       { PRINT, "^(print)" },
    { WHILE, "^(while)" },     { CALL, "^(call)" },
    { IF, "^(if)" },           { THEN, "^(then)" },
    { ELSE, "^(else)" },       { NAME, "^([a-zA-Z]\\w*)\\b" },
    { INTEGER, "^(\\d+)\\b" },
};

std::string_view trimLeadingWhitespace(const std::string_view& s) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::vector<Token> tokenize(std::string content) {
    std::string_view fileView = std::string_view(content);
    std::vector<Token> result;
    fileView = trimLeadingWhitespace(fileView);
    int line = 0;
    int linePos = 0;
    while (fileView.size() > 0) {
        bool matchedSomething = false;
        for (auto const& p : rules) {
            // TODO scrap
            if (p.second.empty()) continue;

            std::match_results<std::string_view::const_iterator> match;
            if (std::regex_search(fileView.begin(), fileView.end(), match, std::regex(p.second))) {

                std::cout << showtype[p.first] << ":" << match.str() << " ";
                if (p.first == LBRACE || p.first == RBRACE || p.first == SEMICOLON)
                    std::cout << "\n";

                Token t(p.first);
                if (p.first == NAME) {
                    t.nameValue = match.str();
                } else if (p.first == INTEGER) {
                    t.integerValue = std::stoi(match.str());
                }
                result.push_back(t);

                matchedSomething = true;
                int push = match.str().size();
                if (push == 0) push = 1;
                fileView = fileView.substr(push);
                break;
            }
        }

        // Move along. TODO this shouldnt be needed.
        if (!matchedSomething) fileView = fileView.substr(1);

        fileView = trimLeadingWhitespace(fileView);
    }
    return result;
}

} // namespace lexer