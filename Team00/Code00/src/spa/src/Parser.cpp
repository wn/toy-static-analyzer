#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Logger.h"
#include "Parser.h"

namespace backend {

// Constructors
Parser::Parser(std::vector<lexer::Token> tokens) : tokens(std::move(tokens)) {
}
State::State(int pos)
: accepted(false), tNode(TNode(TNodeType::INVALID, std::vector<TNode>())), pos(pos){};
State::State(int pos, TNode tNode) : accepted(true), tNode(std::move(tNode)), pos(pos){};

// Parser Helpers
bool Parser::haveTokensLeft(int pos) {
    return pos < tokens.size();
}

lexer::Token Parser::peekToken(int pos) {
    assert(haveTokensLeft(pos));
    return tokens[pos];
}

lexer::Token Parser::popToken(int pos) {
    assert(haveTokensLeft(pos));
    lexer::Token tok = peekToken(pos);
    pos++;
    return tok;
}

bool Parser::tokenTypeIs(int pos, lexer::TokenType type) {
    assert(haveTokensLeft(pos));
    return tokens[pos].type == type;
}


// Parsers
TNode Parser::parse() {
    logLine("Parser: Parsing program");
    return parseProgram(0).tNode;
}

State Parser::parseProgram(int tokenPos) {
    TNode programNode = TNode(TNodeType::Program);
    while (haveTokensLeft(tokenPos)) {
        State procState = parseProcedure(tokenPos);
        if (!procState.accepted) {
            logLine("Parser: Could not parse Program from");
            logWord(procState.pos);
            return State(tokenPos);
        } else {
            logLine("Parser: Procedure AC");
            programNode.children.emplace_back(procState.tNode);
            tokenPos = procState.pos;
        }
    }

    logLine("Parser: Program AC");
    return State(tokenPos, programNode);
}

// Consume a token, return false state with the current tokens if we fail.
#define EAT(tokenType)                                        \
    if (haveTokensLeft(pos) && tokenTypeIs(pos, tokenType)) { \
        popToken(pos);                                        \
    } else                                                    \
        return State(pos);

State Parser::parseProcedure(int tokenPos) {
    if (!tokenTypeIs(tokenPos, lexer::TokenType::PROCEDURE)) {
        return State(tokenPos);
    }
    TNode procedureNode = TNode(TNodeType::Procedure, popToken(tokenPos).line);

    if (!tokenTypeIs(tokenPos, lexer::TokenType::NAME)) {
        return State(tokenPos);
    }
    procedureNode.name = peekToken(tokenPos).nameValue;
    popToken(tokenPos);

    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/48)
    // Parse Statement Lists here instead of consuming everything and
    // making a dummy result.
    while (haveTokensLeft(tokenPos) && !tokenTypeIs(tokenPos, lexer::TokenType::RBRACE))
        popToken(tokenPos);
    assert(tokenTypeIs(tokenPos, lexer::TokenType::RBRACE)), popToken(tokenPos);

    State stmtListResult = State(tokenPos, TNode(TNodeType::StatementList, -1));
    if (stmtListResult.accepted) {
        procedureNode.children.push_back(stmtListResult.tNode);
        return State(stmtListResult.pos, procedureNode);
    } else {
        return stmtListResult;
    }
}
} // namespace backend
