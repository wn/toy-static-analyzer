#include "Parser.h"

#include "Logger.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

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
    const lexer::Token& tok = assertTokenAndPop(tokenPos, lexer::TokenType::NAME);
    if (tok.nameValue != name) {
        throw std::runtime_error("expect " + lexer::prettyPrintType(lexer::TokenType::NAME) + " with value \"" +
                                 name + "\", got " + lexer::prettyPrintType(lexer::TokenType::NAME) +
                                 " with value\"" + tok.nameValue + "\" instead");
    }
    return tok;
}

lexer::Token Parser::assertTokenAndPop(int& tokenPos, lexer::TokenType type) {
    assertTokenIsOfType(tokenPos, type);
    const lexer::Token& tok = peekToken(tokenPos);
    tokenPos++;
    return tok;
}

void Parser::assertTokenIsOfType(int tokenPos, lexer::TokenType type) {
    const lexer::Token& tok = peekToken(tokenPos);
    if (tok.type != type) {
        throw std::runtime_error("expect " + lexer::prettyPrintType(type) + ", got " +
                                 lexer::prettyPrintType(tok.type));
    }
}

bool Parser::tokenTypeIs(int tokenPos, lexer::TokenType type) {
    if (!haveTokensLeft(tokenPos))
        throw std::runtime_error("no more tokens left when trying to tokenTypeIs");
    return tokens[tokenPos].type == type;
}

bool Parser::tokenHasName(int tokenPos, const std::string& name) {
    if (!haveTokensLeft(tokenPos))
        throw std::runtime_error("no more tokens left when trying to tokenHasName");
    const lexer::Token& token = peekToken(tokenPos);
    return token.type == lexer::TokenType::NAME && token.nameValue == name;
}

TNode Parser::parse() {
    logLine("Parser: Parsing program");
    TNode result = parseProgram(0).tNode;
    logLine("Parser: Parsed program completed.");
    return result;
}

State Parser::parseProgram(int tokenPos) {
    logLine("start parseProgram");
    TNode programNode(TNodeType::Program);
    do {
        State procState = parseProcedure(tokenPos);
        programNode.addChild(procState.tNode);
        tokenPos = procState.tokenPos;
    } while (haveTokensLeft(tokenPos));
    logLine("success parseProgram");
    return State(tokenPos, programNode);
}

State Parser::parseProcedure(int tokenPos) {
    logLine("start parseProcedure");
    TNode procedureNode(TNodeType::Procedure,
                        /* line no */ assertNameTokenAndPop(tokenPos, constants::PROCEDURE).line);
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

    do {
        State statementResult = parseStatement(tokenPos);
        statementList.addChild(statementResult.tNode);
        tokenPos = statementResult.tokenPos;
    } while (haveTokensLeft(tokenPos) && !tokenTypeIs(tokenPos, lexer::TokenType::RBRACE));

    assertTokenAndPop(tokenPos, lexer::TokenType::RBRACE);
    logLine("success parseStatementList");
    return State(tokenPos, statementList);
}

// stmt: read | print | call | while | if | assign
State Parser::parseStatement(int tokenPos) {
    logLine("~~in parseStatement~~");
    // Every statement must have at least 2 tokens.
    if (!haveTokensLeft(tokenPos + 1)) {
        throw std::runtime_error(
        "Failed to parse statement: must have at least 2 tokens remaining");
    }
    // If the second token is an equal, we can be sure that this is an assignment statement.
    if (tokenTypeIs(tokenPos + 1, lexer::SINGLE_EQ)) {
        return parseAssign(tokenPos);
    }

    // Ensure that the next token is a name
    assertTokenIsOfType(tokenPos, lexer::NAME);

    // If this is not an assignment statement, it must be another structured statement.
    if (tokenHasName(tokenPos, constants::READ)) {
        return parseRead(tokenPos);
    } else if (tokenHasName(tokenPos, constants::PRINT)) {
        return parsePrint(tokenPos);
    } else if (tokenHasName(tokenPos, constants::CALL)) {
        return parseCall(tokenPos);
    } else if (tokenHasName(tokenPos, constants::WHILE)) {
        return parseWhile(tokenPos);
    } else if (tokenHasName(tokenPos, constants::IF)) {
        return parseIf(tokenPos);
    } else {
        throw std::runtime_error("Failed to parse statement, with first token that has name " +
                                 peekToken(tokenPos).nameValue);
    }
}

State Parser::parseIf(int tokenPos) {
    logLine("start parseIf");
    TNode ifElseNode(TNodeType::IfElse, assertNameTokenAndPop(tokenPos, constants::IF).line);

    assertTokenAndPop(tokenPos, lexer::TokenType::LPAREN);
    const State& conditionalExpressionResult = parseCondition(tokenPos);
    tokenPos = conditionalExpressionResult.tokenPos;
    assertTokenAndPop(tokenPos, lexer::TokenType::RPAREN);
    ifElseNode.addChild(conditionalExpressionResult.tNode);

    assertNameTokenAndPop(tokenPos, constants::THEN);

    const State& thenStatementListResult = parseStatementList(tokenPos);
    tokenPos = thenStatementListResult.tokenPos;
    ifElseNode.addChild(thenStatementListResult.tNode);

    assertNameTokenAndPop(tokenPos, constants::ELSE);

    const State& elseStatementListResult = parseStatementList(tokenPos);
    tokenPos = elseStatementListResult.tokenPos;
    ifElseNode.addChild(elseStatementListResult.tNode);
    logLine("success parseIf");
    return State(tokenPos, ifElseNode);
}

State Parser::parseWhile(int tokenPos) {
    logLine("start parseWhile");
    TNode whileNode(TNodeType::While, assertNameTokenAndPop(tokenPos, constants::WHILE).line);

    assertTokenAndPop(tokenPos, lexer::TokenType::LPAREN);
    const State& condResult = parseCondition(tokenPos);
    tokenPos = condResult.tokenPos;
    assertTokenAndPop(tokenPos, lexer::TokenType::RPAREN);
    whileNode.addChild(condResult.tNode);

    const State& stmtListResult = parseStatementList(tokenPos);
    tokenPos = stmtListResult.tokenPos;
    whileNode.addChild(stmtListResult.tNode);

    logLine("success parseWhile");
    return State(tokenPos, whileNode);
}

// cond_expr: rel_expr
//          | ‘!’ ‘(’ cond_expr ‘)’
//          | ‘(’ cond_expr ‘)’ ‘&&’ ‘(’ cond_expr ‘)’
//          | ‘(’ cond_expr ‘)’ ‘||’ ‘(’ cond_expr ‘)’
State Parser::parseCondition(int tokenPos) {
    logLine("start parseCondition");

    if (peekToken(tokenPos).type == lexer::TokenType::RPAREN) {
        // empty condition.
        throw std::runtime_error("expect condition, none defined");
    }

    // We try to get parseRel. If we cant, then we try the others.
    // Consider ((x + y) < 1) and ((x < 1) && (y < 1))
    // We need to check if it's the first case, before we check if its the second case.
    try {
        State result = parseRelExpr(tokenPos);
        tokenPos = result.tokenPos;
        logLine("success parseCondition");
        return State(tokenPos, result.tNode);
    } catch (const std::exception&) {
        logLine("failed parseRelExpr");
    }

    if (tokenTypeIs(tokenPos, lexer::TokenType::NOT)) {
        // | ‘!’ ‘(’ cond_expr ‘)’
        int line = assertTokenAndPop(tokenPos, lexer::TokenType::NOT).line;
        TNode notNode(TNodeType::Not, line);

        assertTokenAndPop(tokenPos, lexer::TokenType::LPAREN);
        State cond = parseCondition(tokenPos);
        tokenPos = cond.tokenPos;
        assertTokenAndPop(tokenPos, lexer::TokenType::RPAREN);

        notNode.addChild(cond.tNode);
        logLine("success parseCondition");
        return State(tokenPos, notNode);
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::LPAREN)) {
        // | ‘(’ cond_expr ‘)’ ‘&&’ ‘(’ cond_expr ‘)’
        // | ‘(’ cond_expr ‘)’ ‘||’ ‘(’ cond_expr ‘)’
        assertTokenAndPop(tokenPos, lexer::TokenType::LPAREN);
        const State& inner_left_cond = parseCondition(tokenPos);
        tokenPos = inner_left_cond.tokenPos;
        assertTokenAndPop(tokenPos, lexer::TokenType::RPAREN);

        TNodeType type;
        int line;
        if (tokenTypeIs(tokenPos, lexer::TokenType::ANDAND)) {
            type = TNodeType::And;
            line = assertTokenAndPop(tokenPos, lexer::ANDAND).line;
        } else if (tokenTypeIs(tokenPos, lexer::TokenType::OROR)) {
            type = TNodeType::Or;
            line = assertTokenAndPop(tokenPos, lexer::OROR).line;
        } else {
            throw std::runtime_error("expect boolean operator, got " +
                                     prettyPrintType(peekToken(tokenPos).type));
        }
        assertTokenAndPop(tokenPos, lexer::TokenType::LPAREN);
        const State& inner_right_cond = parseCondition(tokenPos);
        tokenPos = inner_right_cond.tokenPos;
        assertTokenAndPop(tokenPos, lexer::TokenType::RPAREN);

        TNode condNode(type, line);
        condNode.addChild(inner_left_cond.tNode);
        condNode.addChild(inner_right_cond.tNode);
        logLine("success parseCondition");
        return State(tokenPos, condNode);
    }
    throw std::runtime_error("Failed to parse conditions");
}

// rel_expr: rel_factor ‘>’ rel_factor
//          | rel_factor ‘>=’ rel_factor
//          | rel_factor ‘<’ rel_factor
//          | rel_factor ‘<=’   rel_factor
//          | rel_factor ‘==’ rel_factor
//          | rel_factor ‘!=’ rel_factor
State Parser::parseRelExpr(int tokenPos) {
    logLine("start parseRelExpr");
    const State& left_rel_factor = parseRelFactor(tokenPos);
    tokenPos = left_rel_factor.tokenPos;

    int line;
    TNodeType opType;
    if (tokenTypeIs(tokenPos, lexer::TokenType::GT)) {
        opType = Greater;
        line = assertTokenAndPop(tokenPos, lexer::TokenType::GT).line;
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::GTE)) {
        opType = GreaterThanOrEqual;
        line = assertTokenAndPop(tokenPos, lexer::TokenType::GTE).line;
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::LT)) {
        opType = Lesser;
        line = assertTokenAndPop(tokenPos, lexer::TokenType::LT).line;
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::LTE)) {
        opType = LesserThanOrEqual;
        line = assertTokenAndPop(tokenPos, lexer::TokenType::LTE).line;
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::EQEQ)) {
        opType = Equal;
        line = assertTokenAndPop(tokenPos, lexer::TokenType::EQEQ).line;
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::NEQ)) {
        opType = NotEqual;
        line = assertTokenAndPop(tokenPos, lexer::TokenType::NEQ).line;
    } else {
        // unexpected operator.
        throw std::runtime_error("unexpected relExpr operator: " +
                                 lexer::prettyPrintType(peekToken(tokenPos).type));
    }

    const State& right_rel_factor = parseRelFactor(tokenPos);
    tokenPos = right_rel_factor.tokenPos;

    TNode relExprNode(opType, line);
    relExprNode.addChild(left_rel_factor.tNode);
    relExprNode.addChild(right_rel_factor.tNode);
    logLine("success parseRelExpr");
    return State(tokenPos, relExprNode);
}

// rel_factor: var_name | const_value | expr
State Parser::parseRelFactor(int tokenPos) {
    logLine("start parseRelFactor");
    State s = parseExpr(tokenPos);
    logLine("success parseRelFactor");
    return s;
}

// expr: expr ‘+’ term | expr ‘-’ term | term
State Parser::parseExpr(int tokenPos) {
    logLine("start parseExpr");
    State result = parseTerm(tokenPos);
    tokenPos = result.tokenPos;
    while (haveTokensLeft(tokenPos)) {
        lexer::TokenType nextType = peekToken(tokenPos).type;
        if (nextType != lexer::TokenType::PLUS && nextType != lexer::TokenType::MINUS) {
            break;
        }
        auto operatorToken = assertTokenAndPop(tokenPos, nextType);

        // Construct a new TNode as the result, like so:
        //     newExpr: +
        //            /   \
        //        result   nextTerm
        // This results in left-associativity, as the new term is the right-child.
        State nextTermState = parseTerm(tokenPos);

        // Create newExpr
        TNodeType newExprType = operatorToken.type == lexer::PLUS ? TNodeType::Plus : TNodeType::Minus;
        TNode newExpr(newExprType);
        newExpr.addChild(result.tNode);
        newExpr.addChild(nextTermState.tNode);

        // Update the result;
        result = State(nextTermState.tokenPos, newExpr);
        tokenPos = result.tokenPos;
    }
    return result;
}

// term: term ‘*’ factor | term ‘/’ factor | term ‘%’ factor | factor
State Parser::parseTerm(int tokenPos) {
    logLine("start parseTerm");
    State result = parseFactor(tokenPos);
    tokenPos = result.tokenPos;

    std::map<lexer::TokenType, TNodeType> tokenTypeToTNodeType = {
        { lexer::TokenType::MULT, TNodeType::Multiply },
        { lexer::TokenType::DIV, TNodeType::Divide },
        { lexer::TokenType::MOD, TNodeType::Modulo }
    };
    while (haveTokensLeft(tokenPos)) {
        lexer::TokenType nextType = peekToken(tokenPos).type;
        if (nextType != lexer::TokenType::MULT && nextType != lexer::TokenType::DIV &&
            nextType != lexer::TokenType::MOD) {
            break;
        }
        auto operatorToken = assertTokenAndPop(tokenPos, nextType);

        // Construct a new TNode as the result, like so:
        //     newTerm: *
        //            /   \
        //        result   nextFactor
        // This results in left-associativity, as the new factor is the right-child.
        State nextFactorState = parseFactor(tokenPos);

        // Create newExpr
        TNodeType newTermType = tokenTypeToTNodeType[operatorToken.type];
        TNode newTerm(newTermType);
        newTerm.addChild(result.tNode);
        newTerm.addChild(nextFactorState.tNode);

        // Update the result;
        result = State(nextFactorState.tokenPos, newTerm);
        tokenPos = result.tokenPos;
    }
    logLine("success parseTerm");
    return result;
}

// factor: var_name | const_value | ‘(’ expr ‘)’
State Parser::parseFactor(int tokenPos) {
    logLine("start parseFactor");
    if (tokenTypeIs(tokenPos, lexer::TokenType::NAME)) {
        State s = parseVarName(tokenPos);
        logLine("success parseFactor");
        return s;
    } else if (tokenTypeIs(tokenPos, lexer::TokenType::INTEGER)) {
        State s = parseConstValue(tokenPos);
        logLine("success parseFactor");
        return s;
    } else {
        assertTokenAndPop(tokenPos, lexer::TokenType::LPAREN);
        State res = parseExpr(tokenPos);
        tokenPos = res.tokenPos;
        assertTokenAndPop(tokenPos, lexer::TokenType::RPAREN);
        logLine("success parseFactor");
        return State(tokenPos, res.tNode);
    }
}

// var_name, proc_name: NAME
State Parser::parseVarName(int tokenPos) {
    logLine("start parseVarName");
    const lexer::Token& t = assertTokenAndPop(tokenPos, lexer::TokenType::NAME);
    TNode node(Variable, t.line);
    node.name = t.nameValue;
    logLine("success parseVarName");
    return State(tokenPos, node);
}

// const_value: INTEGER
State Parser::parseConstValue(int tokenPos) {
    logLine("start parseConstValue");
    const lexer::Token& t = assertTokenAndPop(tokenPos, lexer::TokenType::INTEGER);
    TNode node(Constant, t.line);
    node.constant = t.integerValue;
    logLine("success parseConstValue");
    return State(tokenPos, node);
}

// assign: var_name ‘=’ expr ‘;’
State Parser::parseAssign(int tokenPos) {
    logLine("start parseAssign");

    const State& lhsState = parseVarName(tokenPos);
    tokenPos = lhsState.tokenPos;
    const TNode& lhsNode = lhsState.tNode;

    int eqLine = assertTokenAndPop(tokenPos, lexer::TokenType::SINGLE_EQ).line;
    TNode assignNode(TNodeType::Assign, eqLine);

    const State& exprState = parseExpr(tokenPos);
    tokenPos = exprState.tokenPos;
    assignNode.addChild(lhsNode);
    assignNode.addChild(exprState.tNode);

    assertTokenAndPop(tokenPos, lexer::TokenType::SEMICOLON);

    logLine("success parseAssign");
    return State(tokenPos, assignNode);
}

// read: ‘read’ var_name’;’
State Parser::parseRead(int tokenPos) {
    const lexer::Token& token = assertNameTokenAndPop(tokenPos, constants::READ);
    TNode readNode(Read, token.line);

    const State& varState = parseVarName(tokenPos);
    tokenPos = varState.tokenPos;

    readNode.addChild(varState.tNode);
    assertTokenAndPop(tokenPos, lexer::TokenType::SEMICOLON);
    return State(tokenPos, readNode);
}

// print: ‘print’ var_name’;’
State Parser::parsePrint(int tokenPos) {
    const lexer::Token& token = assertNameTokenAndPop(tokenPos, constants::PRINT);
    TNode printNode(Print, token.line);

    const State& varState = parseVarName(tokenPos);
    tokenPos = varState.tokenPos;

    printNode.addChild(varState.tNode);
    assertTokenAndPop(tokenPos, lexer::TokenType::SEMICOLON);
    return State(tokenPos, printNode);
}

// call: ‘call’ proc_name ‘;’
State Parser::parseCall(int tokenPos) {
    const lexer::Token& token = assertNameTokenAndPop(tokenPos, constants::CALL);
    TNode callNode(Call, token.line);

    const State& varState = parseVarName(tokenPos);
    tokenPos = varState.tokenPos;
    TNode callVar(std::move(varState.tNode));
    callVar.isProcedureVar = true;

    callNode.addChild(callVar);
    assertTokenAndPop(tokenPos, lexer::TokenType::SEMICOLON);
    return State(tokenPos, callNode);
}

/**
 *
 * @param exprStr
 * @return the expr string with precedence bracket. Return empty string if there is any error.
 */
std::string Parser::parseExpr(const std::string& exprStr) {
    try {
        std::istringstream iStr(exprStr);
        // If exprStr is invalid, tokenize will throw.
        std::vector<lexer::Token> tokens = lexer::tokenize(iStr);

        Parser parser(tokens);
        // If there is any issue with parsing tokens, parseExpr will throw.
        State s = parser.parseExpr(0);
        return getExprString(s.tNode);
    } catch (const std::exception& e) {
        return "";
    }
}

bool Parser::isValidExpr(const std::string& exprStr) {
    return !Parser::parseExpr(exprStr).empty();
}

} // namespace backend
