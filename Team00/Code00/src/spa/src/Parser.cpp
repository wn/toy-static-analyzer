#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Logger.h"
#include "Parser.h"

namespace backend {
Parser::Parser(std::vector<lexer::Token> tokens) : tokens(std::move(tokens)) {
}
State::State(int tokenPos, TNode tNode) : tNode(std::move(tNode)), tokenPos(tokenPos){};

bool Parser::haveTokensLeft(int tokenPos) const {
    return tokenPos < tokens.size();
}

lexer::Token Parser::peekToken(int tokenPos) {
    assert(haveTokensLeft(tokenPos));
    return tokens[tokenPos];
}

lexer::Token Parser::assertTokenAndPop(int& tokenPos, lexer::TokenType type) {
    assert(haveTokensLeft(tokenPos));
    if (tokens[tokenPos].type != type) {
        throw std::runtime_error("expect " + lexer::prettyPrintType(type) + ", got " +
                                 lexer::prettyPrintType(tokens[tokenPos].type));
    }
    lexer::Token tok = peekToken(tokenPos);
    tokenPos++;
    return tok;
}

bool Parser::tokenTypeIs(int pos, lexer::TokenType type) {
    assert(haveTokensLeft(pos));
    return tokens[pos].type == type;
}

TNode Parser::parse() {
    logLine("Parser: Parsing program");
    auto result = parseProgram(0).tNode;
    logLine("Parser: Parsed program completed.");
    return result;
}

State Parser::parseProgram(int tokenPos) {
    logLine("start parseProgram");
    TNode programNode = TNode(TNodeType::Program);
    while (haveTokensLeft(tokenPos)) {
        State procState = parseProcedure(tokenPos);
        programNode.addChild(procState.tNode);
        tokenPos = procState.tokenPos;
    }
    logLine("success parseProgram");
    return State(tokenPos, programNode);
}

State Parser::parseProcedure(int tokenPos) {
    logLine("start parseProcedure");
    TNode procedureNode =
    TNode(TNodeType::Procedure, assertTokenAndPop(tokenPos, lexer::TokenType::PROCEDURE).line);
    procedureNode.name = assertTokenAndPop(tokenPos, lexer::TokenType::NAME).nameValue;

    State stmtListResult = parseStatementList(tokenPos);
    procedureNode.children.push_back(stmtListResult.tNode);
    logLine("success parseProcedure");
    return State(stmtListResult.tokenPos, procedureNode);
}

State Parser::parseStatementList(int tokenPos) {
    logLine("start parseStatementList");
    TNode statementList(TNodeType::StatementList,
                        assertTokenAndPop(tokenPos, lexer::TokenType::LBRACE).line);

    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/48)
    // don't consume until RBRACE here
    while (haveTokensLeft(tokenPos) && !tokenTypeIs(tokenPos, lexer::TokenType::RBRACE)) {
        tokenPos++;
    }

    assertTokenAndPop(tokenPos, lexer::TokenType::RBRACE);
    logLine("success parseStatementList");
    return State(tokenPos, statementList);
}
} // namespace backend
