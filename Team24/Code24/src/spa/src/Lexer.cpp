#define DEBUG 0

#include "Lexer.h"

#include <cctype>

namespace backend {
namespace lexer {

std::string prettyPrintType(TokenType t) {
    std::map<TokenType, std::string> m = {
        { LBRACE, "LBRACE" },
        { RBRACE, "RBRACE" },
        { LPAREN, "LPAREN" },
        { RPAREN, "RPAREN" },
        { SEMICOLON, "SEMICOLON" },
        { COMMA, "COMMA" },
        { UNDERSCORE, "UNDERSCORE" },
        { DOUBLE_QUOTE, "DOUBLE_QUOTE" },
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
        { NAME, "NAME" },
        { INTEGER, "INTEGER" },
        { WHITESPACE, "WHITESPACE" },
    };
    return m[t];
}

std::vector<std::pair<TokenType, std::string>> rules = { { LBRACE, "^(\\{)" },
                                                         { RBRACE, "^(\\})" },
                                                         { LPAREN, "^(\\()" },
                                                         { RPAREN, "^(\\))" },
                                                         { SEMICOLON, "^(;)" },
                                                         { COMMA, "^(,)" },
                                                         { UNDERSCORE, "^(_)" },
                                                         { DOUBLE_QUOTE, "^(\")" },

                                                         { NEQ, "^(!=)" },
                                                         { NOT, "^(!)" },
                                                         { EQEQ, "^(==)" },
                                                         { SINGLE_EQ, "^(=)" },

                                                         { ANDAND, "^(&&)" },
                                                         { OROR, "^(\\|\\|)" },
                                                         { GTE, "^(>=)" },
                                                         { GT, "^(>)" },

                                                         { LTE, "^(<=)" },
                                                         { LT, "^(<)" },

                                                         { PLUS, "^(\\+)" },
                                                         { MINUS, "^(-)" },
                                                         { MULT, "^(\\*)" },
                                                         { DIV, "^(\\/)" },
                                                         { MOD, "^(%)" },

                                                         { NAME, "^([a-zA-Z]\\w*)\\b" },

                                                         // Captures any set of numbers.
                                                         // Ensure that trailing zeroes are
                                                         // not allowed when using this rule.
                                                         { INTEGER, "^(\\d+)" },

                                                         { WHITESPACE, "^(\\s+)" } };


// Splits input into lines. getline has the added benefit of dealing with
// platform-specific newline interpretations.
std::vector<std::string> splitLines(std::istream& stream) {
    std::vector<std::string> result;
    std::string line;
    while (std::getline(stream, line)) {
        result.push_back(line);
    }
    return result;
}


std::vector<Token> tokenize(std::istream& stream, bool willLexWithWhitespace) {
    std::vector<Token> result;

    int lineNumber = 1;
    std::vector<std::string> lines = splitLines(stream);

    for (; lineNumber <= lines.size(); lineNumber++) {
        std::string line = lines[lineNumber - 1];
        std::string originalLine = line.substr();
        while (!line.empty()) {
            bool matchedSomething = false;
            for (auto const& p : rules) {
                std::smatch match;
                if (std::regex_search(line, match, std::regex(p.second))) {

                    Token t(p.first);
                    t.line = lineNumber;
                    t.linePosition = (int)(originalLine.size() - line.size());
                    if (p.first == NAME) {
                        t.nameValue = match.str();
                    } else if (p.first == INTEGER) {
                        t.integerValue = match.str();
                        // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/173):
                        // Integers cannot be '00001' Which is permissible by this rule.
                        if (t.integerValue[0] == '0' && t.integerValue.size() > 1) {
                            throw std::runtime_error("Trailing zeroes not allowed: " + t.integerValue);
                        }
                    }

                    result.push_back(t);

                    if (DEBUG) {
                        std::cout << prettyPrintType(p.first) << "<" << t.line << ", " << t.linePosition << ">";
                        if (p.first == NAME || p.first == INTEGER) std::cout << ":" << match.str();
                        std::cout << " ";
                        if (p.first == LBRACE || p.first == RBRACE || p.first == SEMICOLON)
                            std::cout << "\n";
                    }

                    matchedSomething = true;
                    line = line.substr(match.str().size());
                    break;
                }
            }

            // No valid matches.
            if (!matchedSomething) {
                throw std::runtime_error(
                "Lexer: No rules available to parse the remaining line: <" + line +
                "> at line: " + std::to_string(lineNumber));
            }
        }

        // Feed a newline token at the end of every (non-last) line.
        if (lineNumber != lines.size()) {
            Token newLine(WHITESPACE);
            newLine.line = lineNumber;
            result.push_back(newLine);
        }
    }

    // Compress whitespaces together. For e.g. " \n " will be stored as 3 consecutive whitespaces.
    std::vector<Token> resultWithoutConsecutiveWhitespace;
    for (Token& token : result) {
        if (token.type != TokenType::WHITESPACE) {
            resultWithoutConsecutiveWhitespace.push_back(token);
        } else if (!resultWithoutConsecutiveWhitespace.empty() &&
                   resultWithoutConsecutiveWhitespace.back().type != TokenType::WHITESPACE) {
            resultWithoutConsecutiveWhitespace.push_back(token);
        }
    }
    result = resultWithoutConsecutiveWhitespace;

    // Early return if we want to return whitespace
    if (willLexWithWhitespace) {
        return result;
    }

    std::vector<Token> resultWithoutWhitespace;
    for (auto& t : result) {
        if (t.type != WHITESPACE) {
            resultWithoutWhitespace.push_back(t);
        }
    }
    return resultWithoutWhitespace;
}

// Public API definition

std::vector<Token> tokenize(std::istream& stream) {
    return tokenize(stream, false);
};

std::vector<Token> tokenizeWithWhitespace(std::istream& stream) {
    return tokenize(stream, true);
};

} // namespace lexer
} // namespace backend
