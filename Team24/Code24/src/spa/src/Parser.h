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

namespace constants {
constexpr char PROCEDURE[] = "procedure";
constexpr char READ[] = "read";
constexpr char PRINT[] = "print";
constexpr char WHILE[] = "while";
constexpr char CALL[] = "call";
constexpr char IF[] = "if";
constexpr char THEN[] = "then";
constexpr char ELSE[] = "else";
} // namespace constants

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
    bool tokenHasName(int tokenPos, const std::string& name);
    lexer::Token peekToken(int tokenPos);
    lexer::Token assertTokenAndPop(int& tokenPos, lexer::TokenType);
    lexer::Token assertNameTokenAndPop(int& tokenPos, const std::string& name);

    // -- Parser primitives --
    State parseProgram(int tokenPos);
    State parseProcedure(int tokenPos);

    // Stmt
    State parseStatementList(int tokenPos);
    State parseStatement(int tokenPos);
    State parseIf(int tokenPos);
    State parseWhile(int tokenPos);
    State parseCondition(int tokenPos);
    State parseAssign(int tokenPos);

    // cond
    State parseRelExpr(int tokenPos);
    State parseRelFactor(int tokenPos);
    State parseExpr(int tokenPos);
    State parseTerm(int tokenPos);
    State parseFactor(int tokenPos);
    State parseConstValue(int tokenPos);
    State parseVarName(int tokenPos);

    State parseRead(int tokenPos);
    State parsePrint(int tokenPos);
    State parseCall(int tokenPos);
};
} // namespace backend
