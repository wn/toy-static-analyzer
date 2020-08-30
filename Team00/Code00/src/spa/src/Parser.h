#pragma once

#include "Lexer.h"
#include "TNode.h"
#include <list>
#include <vector>

namespace backend {
class State {
  public:
    explicit State(int pos);
    explicit State(int pos, TNode tNode);
    // Used to check if parsing was successful
    bool accepted;
    // Created tNode, if accepted
    TNode tNode;
    // ending position
    int pos{ -1 };
};

class Parser {
  public:
    explicit Parser(std::vector<lexer::Token> tokens);
    // Generate AST from parser.
    TNode parse();

  private:
    std::vector<lexer::Token> tokens;
    // Helpers
    bool haveTokensLeft(int pos);
    bool tokenTypeIs(int pos, lexer::TokenType);
    lexer::Token peekToken(int pos);
    lexer::Token popToken(int pos);

    // Parser primitives
    State parseProgram(int tokenPos);
    State parseProcedure(int tokenPos);
};
} // namespace backend
