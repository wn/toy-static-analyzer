#include "Parser.h"

#include "catch.hpp"
namespace backend {
namespace testparser {
void require(bool b) {
    REQUIRE(b);
}

Parser GenerateParserFromTokens(const std::string& expr) {
    std::istringstream iStr(expr);
    return Parser(lexer::tokenize(iStr));
}

TNode generateProgramNodeFromStatement(const std::string& name, const TNode& node) {
    TNode stmtNode(TNodeType::StatementList, 1);
    stmtNode.addChild(node);

    TNode procNode(TNodeType::Procedure, 1, name);
    procNode.addChild(stmtNode);

    TNode progNode(TNodeType::Program, -1);
    progNode.addChild(procNode);
    return progNode;
}


TEST_CASE("Test parseStatementList fails on 0 statements") {
    Parser parser = GenerateParserFromTokens("procedure p{}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got RBRACE");
}

TEST_CASE("Test procedure with name 'procedure'") {
    Parser parser = GenerateParserFromTokens("procedure procedure {y = 1 + 1;}");
    REQUIRE_NOTHROW(parser.parse());
}

TEST_CASE("Test parseAssign") {
    Parser parser = GenerateParserFromTokens("procedure p{y = 1 + 1;}");
    TNode result = parser.parse();

    TNode stmt(TNodeType::Assign, 1);
    stmt.name = "y";
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/64):
    stmt.addChild(TNode(TNodeType::INVALID));
    stmt.addChild(TNode(TNodeType::INVALID));
    stmt.addChild(TNode(TNodeType::INVALID));

    require(result == generateProgramNodeFromStatement("p", stmt));
}
} // namespace testparser
} // namespace backend
