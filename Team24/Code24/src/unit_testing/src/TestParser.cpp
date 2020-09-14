#include "Parser.h"
#include <sstream>

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

    TNode procNode(TNodeType::Procedure, 1);
    procNode.name = name;
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

TEST_CASE("Test parseIf") {
    Parser parser =
    GenerateParserFromTokens("procedure p{ if (xoxo == 1) then {x=2;} else {x=2;}}");
    REQUIRE_NOTHROW(parser.parse());

    // The Parser is currently expected to parse the empty condition, `()`, without error,
    // this will change when expressions can be parsed, upon which we should expect an error.
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/49)
    parser = GenerateParserFromTokens("procedure p{ if () then {x=2;} else {x=2;}}");
    REQUIRE_NOTHROW(parser.parse());

    // No "then"
    parser = GenerateParserFromTokens("procedure p{ if (xoxo == 1) {x=2;} else {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got LBRACE");

    // No "else"
    parser = GenerateParserFromTokens("procedure p{ if (xoxo == 1) then {x=2;} {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got LBRACE");

    // empty statement list
    parser = GenerateParserFromTokens("procedure p{ if (xoxo == 1) then {} else {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got RBRACE");
}

TEST_CASE("Test nested while and if statements") {
    // While inside If
    Parser parser = GenerateParserFromTokens("procedure p{ if (xoxo == 1) "
                                             "then {while (y==3) {x = 5;}} "
                                             "else {while (y==3) {x = 5;}}}");
    REQUIRE_NOTHROW(parser.parse());

    // If inside While
    parser = GenerateParserFromTokens("procedure p{ while (xoxo == 1) "
                                      "{if (y==3) then {z = 8;} else {y = 3;}}}");
    REQUIRE_NOTHROW(parser.parse());
}


TEST_CASE("Test parseWhile") {
    Parser parser = GenerateParserFromTokens("procedure p{ while (xoxo == 1) {x=2;}}");
    REQUIRE_NOTHROW(parser.parse());

    // The Parser is currently expected to parse the empty condition, `()`, without error,
    // this will change when expressions can be parsed, upon which we should expect an error.
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/49)
    parser = GenerateParserFromTokens("procedure p{ while () {x=2;}}");
    REQUIRE_NOTHROW(parser.parse());

    // empty statement list
    parser = GenerateParserFromTokens("procedure p{ while (xoxo == 1) {}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got RBRACE");
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
