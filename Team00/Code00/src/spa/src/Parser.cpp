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
    if (!haveTokensLeft(tokenPos))
        throw std::runtime_error("no more tokens left when trying to peekToken");
    return tokens[tokenPos];
}

lexer::Token Parser::assertNameTokenAndPop(int& tokenPos, const std::string& name) {
    if (!haveTokensLeft(tokenPos))
        throw std::runtime_error("no more tokens left when trying to assertNameTokenAndPop");
    if (tokens[tokenPos].type != lexer::TokenType::NAME) {
        throw std::runtime_error("expect " + lexer::prettyPrintType(lexer::TokenType::NAME) + " with value \"" +
                                 name + "\", got " + lexer::prettyPrintType(tokens[tokenPos].type));
    }
    lexer::Token tok(std::move(peekToken(tokenPos)));
    if (tok.nameValue.empty() || tok.nameValue != name) {
        throw std::runtime_error("expect " + lexer::prettyPrintType(lexer::TokenType::NAME) +
                                 " with value \"" + name + "\", got \"" + name + "\" instead");
    }
    tokenPos++;
    return tok;
}

lexer::Token Parser::assertTokenAndPop(int& tokenPos, lexer::TokenType type) {
    if (!haveTokensLeft(tokenPos))
        throw std::runtime_error("no more tokens left when trying to assertTokenAndPop");
    if (tokens[tokenPos].type != type) {
        throw std::runtime_error("expect " + lexer::prettyPrintType(type) + ", got " +
                                 lexer::prettyPrintType(tokens[tokenPos].type));
    }
    lexer::Token tok(std::move(peekToken(tokenPos)));
    tokenPos++;
    return tok;
}

bool Parser::tokenTypeIs(int tokenPos, lexer::TokenType type) {
    if (!haveTokensLeft(tokenPos))
        throw std::runtime_error("no more tokens left when trying to tokenTypeIs");
    return tokens[tokenPos].type == type;
}

TNode Parser::parse() {
    logLine("Parser: Parsing program");
    TNode result(std::move(parseProgram(0).tNode));
    logLine("Parser: Parsed program completed.");
    return result;
}

State Parser::parseProgram(int tokenPos) {
    logLine("start parseProgram");
    TNode programNode(TNodeType::Program);
    while (haveTokensLeft(tokenPos)) {
        State procState(std::move(parseProcedure(tokenPos)));
        programNode.addChild(procState.tNode);
        tokenPos = procState.tokenPos;
    }
    logLine("success parseProgram");
    return State(tokenPos, programNode);
}

State Parser::parseProcedure(int tokenPos) {
    logLine("start parseProcedure");
    TNode procedureNode(TNodeType::Procedure,
                        /* line no */ assertNameTokenAndPop(tokenPos, constants::PROCEDURE).line,
                        /* name */ assertTokenAndPop(tokenPos, lexer::TokenType::NAME).nameValue);

    State stmtListResult(std::move(parseStatementList(tokenPos)));
    procedureNode.children.push_back(stmtListResult.tNode);
    logLine("success parseProcedure");
    return State(stmtListResult.tokenPos, procedureNode);
}

State Parser::parseStatementList(int tokenPos) {
    logLine("start parseStatementList");
    TNode statementList(TNodeType::StatementList,
                        assertTokenAndPop(tokenPos, lexer::TokenType::LBRACE).line);

    do {
        State statementResult(std::move(parseStatement(tokenPos)));
        statementList.addChild(statementResult.tNode);
        tokenPos = statementResult.tokenPos;
    } while (haveTokensLeft(tokenPos) && !tokenTypeIs(tokenPos, lexer::TokenType::RBRACE));

    assertTokenAndPop(tokenPos, lexer::TokenType::RBRACE);
    logLine("success parseStatementList");
    return State(tokenPos, statementList);
}

State Parser::parseStatement(int tokenPos) {
    logLine("start parseStatement");
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/48)
    // Implement call, print, read. If any of these is called, code will default to parseAssign.
    logLine("Successfully parsed a statement for a statementlist.");
    return parseAssign(tokenPos);
}

State Parser::parseAssign(int tokenPos) {
    logLine("start parseAssign");

    lexer::Token nameToken(std::move(assertTokenAndPop(tokenPos, lexer::TokenType::NAME)));
    TNode assignNode(TNodeType::Assign, nameToken.line, nameToken.nameValue);

    assertTokenAndPop(tokenPos, lexer::TokenType::SINGLE_EQ);

    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/64):
    while (haveTokensLeft(tokenPos) && !tokenTypeIs(tokenPos, lexer::TokenType::SEMICOLON)) {
        TNode child(TNodeType::INVALID);
        assignNode.addChild(child);
        tokenPos++;
    }

    assertTokenAndPop(tokenPos, lexer::TokenType::SEMICOLON);

    logLine("success parseAssign");
    return State(tokenPos, assignNode);
}
} // namespace backend
