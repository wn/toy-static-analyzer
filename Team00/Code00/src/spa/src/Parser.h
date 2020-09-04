#pragma once

#include "Lexer.h"
#include "TNode.h"
#include <list>
#include <vector>

namespace backend {
class State {
  public:
    explicit State(int tokenPos, TNode tNode);
    // Created tNode
    TNode tNode;
    // ending position
    int tokenPos{ -1 };
};

class Parser {
  public:
    explicit Parser(std::vector<lexer::Token> tokens);
    // Generate AST from parser.
    TNode parse();

    std::vector<lexer::Token> tokens;
    // -- Helpers --

    // Returns true if there are any tokens left in the
    // Parser's vector of tokens. tokenPos is 0-indexed.
    bool haveTokensLeft(int tokenPos) const;
    bool tokenTypeIs(int tokenPos, lexer::TokenType);
    lexer::Token peekToken(int tokenPos);
    lexer::Token assertTokenAndPop(int& tokenPos, lexer::TokenType);

    // -- Parser primitives --
    State parseProgram(int tokenPos);
    State parseProcedure(int tokenPos);

    // Stmt
    State parseStatementList(int tokenPos);
};
} // namespace backend
