#include "Logger.h"
#include "Parser.h"
#include "TestParserHelpers.h"
#include "catch.hpp"

#include <sstream>

namespace backend {
namespace testparser {

TEST_CASE("Test parseStatementList fails on 0 statements") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{}");
    REQUIRE_THROWS_WITH(parser.parse(),
                        "Failed to parse statement: must have at least 2 tokens remaining");
}

TEST_CASE("Test procedure with name 'procedure'") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure procedure {y = 1 + 1;}");
    REQUIRE_NOTHROW(parser.parse());
}

TEST_CASE("Test parseIf") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{ if (xoxo == 1) then {x=2;} else {x=2;}}");
    REQUIRE_NOTHROW(parser.parse());

    parser = testhelpers::GenerateParserFromTokens("procedure p{ if () then {x=2;} else {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect condition, none defined");

    // No "then"
    parser =
    testhelpers::GenerateParserFromTokens("procedure p{ if (xoxo == 1) {x=2;} else {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got LBRACE");

    // No "else"
    parser =
    testhelpers::GenerateParserFromTokens("procedure p{ if (xoxo == 1) then {x=2;} {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got LBRACE");

    // empty statement list
    parser =
    testhelpers::GenerateParserFromTokens("procedure p{ if (xoxo == 1) then {} else {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got RBRACE");
}

TEST_CASE("Test nested while and if statements") {
    // While inside If
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{ if (xoxo == 1) "
                                                          "then {while (y==3) {x = 5;}} "
                                                          "else {while (y==3) {x = 5;}}}");
    REQUIRE_NOTHROW(parser.parse());

    // If inside While
    parser = testhelpers::GenerateParserFromTokens("procedure p{ while (xoxo == 1) "
                                                   "{if (y==3) then {z = 8;} else {y = 3;}}}");
    REQUIRE_NOTHROW(parser.parse());
}


TEST_CASE("Test parseWhile") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{ while (xoxo == 1) {x=2;}}");
    REQUIRE_NOTHROW(parser.parse());

    parser = testhelpers::GenerateParserFromTokens("procedure p{ while () {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect condition, none defined");

    // empty statement list
    parser = testhelpers::GenerateParserFromTokens("procedure p{ while (xoxo == 1) {}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got RBRACE");
}

TEST_CASE("Test Condition with <= math operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z <= 1){y = y + 1;}}");
    TNode result = parser.parse();

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);
    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with >= math operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z >= 1){y = y + 1;}}");
    TNode result = parser.parse();

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);
    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(GreaterThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with > math operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z > 1){y = y + 1;}}");
    TNode result = parser.parse();

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);
    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(Greater, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with < math operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z < 1){y = y + 1;}}");
    TNode result = parser.parse();

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);
    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(Lesser, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with == math operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z == 1){y = y + 1;}}");
    TNode result = parser.parse();

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);
    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(Equal, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with != math operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z != 1) {y = y + 1;}}");
    TNode result = parser.parse();

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);
    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(NotEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with AND operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while ((x < 1) && (y < 1)){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = "1";
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = "1";
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(And, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with OR operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while ((x < 1) || (y < 1)){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = "1";
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = "1";
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(Or, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with NOT operator") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (!((x < 1) || (y < 1))){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = "1";
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = "1";
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(Or, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    TNode notNode(Not, 1);
    notNode.addChild(condNode);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(notNode));
}

TEST_CASE("Test Condition with Triple operator") {
    Parser parser = testhelpers::GenerateParserFromTokens(
    "procedure p{while (!(((x < 1) || (y < 1)) && ((x < 1) || (y < 1)))){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = "1";
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = "1";
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(Or, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    // condNode is ((x < 1) || (y < 1))
    TNode andNode(And, 1);
    // Since left and right of AND is symmetric, we add condNode to AND twice.
    andNode.addChild(condNode);
    andNode.addChild(condNode);

    TNode notNode(Not, 1);
    notNode.addChild(andNode);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(notNode));
}

TEST_CASE("Test expr precedence: + before *") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y * z <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //         +
    //        /  \
    //       x    *
    //           /  \
    //          y    z

    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";
    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(y);
    lCondLHSMultiply.addChild(z);

    TNode x(Variable, 1);
    x.name = "x";
    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr precedence: * before +") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x * y + z <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //         +
    //        /  \
    //       *    z
    //      /  \
    //     x    y

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";

    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(x);
    lCondLHSMultiply.addChild(y);

    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(lCondLHSMultiply);
    lCondLHSPlus.addChild(z);

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << testhelpers::generateProgramNodeFromCondition(condNode).toString() << std::endl;
    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr left-associativity of +") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x + y + z <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //               +
    //             /   \
    //            +     z
    //           /  \
    //          x    y

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";

    TNode lCondLHSMultiply2(Plus);
    lCondLHSMultiply2.addChild(x);
    lCondLHSMultiply2.addChild(y);

    TNode lCondLHSMultiply(Plus);
    lCondLHSMultiply.addChild(lCondLHSMultiply2);
    lCondLHSMultiply.addChild(z);

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << testhelpers::generateProgramNodeFromCondition(condNode).toString() << std::endl;
    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr left-associativity of *") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x * y * z <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //               *
    //             /   \
    //            *     z
    //           /  \
    //          x    y

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";

    TNode lCondLHSMultiply2(Multiply);
    lCondLHSMultiply2.addChild(x);
    lCondLHSMultiply2.addChild(y);

    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(lCondLHSMultiply2);
    lCondLHSMultiply.addChild(z);

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << testhelpers::generateProgramNodeFromCondition(condNode).toString() << std::endl;
    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr precedence: * before + with brackets") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while (x * (y + z) <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //         *
    //        /  \
    //       x    +
    //           /  \
    //          y    z

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";

    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(y);
    lCondLHSPlus.addChild(z);

    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(x);
    lCondLHSMultiply.addChild(lCondLHSPlus);

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << testhelpers::generateProgramNodeFromCondition(condNode).toString() << std::endl;
    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr precedence: + before * with brackets") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while ((x + y) * z <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //         *
    //        /  \
    //       +    z
    //      /  \
    //     x    y

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";

    TNode lCondLHSPlus(Plus);
    lCondLHSPlus.addChild(x);
    lCondLHSPlus.addChild(y);

    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(lCondLHSPlus);
    lCondLHSMultiply.addChild(z);

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << testhelpers::generateProgramNodeFromCondition(condNode).toString() << std::endl;
    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr useless brackets") {
    Parser parser =
    testhelpers::GenerateParserFromTokens("procedure p{while ((x) <= (1)){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(x);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = "1";
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << testhelpers::generateProgramNodeFromCondition(condNode).toString() << std::endl;
    REQUIRE(result == testhelpers::generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test condition, throw properly") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{while (+++){y = y + 1;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "Failed to parse conditions");
}

TEST_CASE("Test parseAssign") {
    // Note: since expr has been tested in parseCondition, we did not write extensive test for
    // the RHS of assign.
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{x = y + 1;}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //         :stmtlst
    //            |
    //         :assign
    //          /  \
    //         x    +
    //             /  \
    //            y    1

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";

    TNode constNode(Constant, 1);
    constNode.constant = "1";

    TNode plusNode(Plus);
    plusNode.addChild(y);
    plusNode.addChild(constNode);

    TNode assignNode(Assign, 1);
    assignNode.addChild(x);
    assignNode.addChild(plusNode);

    REQUIRE(result == testhelpers::generateProgramNodeFromStatement("p", assignNode));
}

TEST_CASE("Test parseAssign wrong LHS") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{x + y = y + 1;}");
    REQUIRE_THROWS_WITH(parser.parse(),
                        "Failed to parse statement, with first token that has name x");
}

TEST_CASE("Test read") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{read x;}");
    TNode result = parser.parse();

    // We expect our node to look like:
    //         read
    //           |
    //           x
    TNode x(Variable, 1);
    x.name = "x";
    TNode readNode(Read, 1);
    readNode.addChild(x);

    REQUIRE(result == testhelpers::generateProgramNodeFromStatement("p", readNode));

    Parser parser_err = testhelpers::GenerateParserFromTokens("procedure p{read x + y;}");
    REQUIRE_THROWS_WITH(parser_err.parse(), "expect SEMICOLON, got PLUS");
}

TEST_CASE("Test print") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{print x;}");
    TNode result = parser.parse();

    // We expect our node to look like:
    //         print
    //           |
    //           x
    TNode x(Variable, 1);
    x.name = "x";
    TNode printNode(Print, 1);
    printNode.addChild(x);

    REQUIRE(result == testhelpers::generateProgramNodeFromStatement("p", printNode));

    Parser parser_err = testhelpers::GenerateParserFromTokens("procedure p{print x + y;}");
    REQUIRE_THROWS_WITH(parser_err.parse(), "expect SEMICOLON, got PLUS");
}

TEST_CASE("Test call") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure p{call x;}");
    TNode result = parser.parse();

    // We expect our node to look like:
    //         call
    //           |
    //           x
    TNode x(Variable, 1);
    x.name = "x";
    TNode callNode(Call, 1);
    callNode.addChild(x);

    REQUIRE(result == testhelpers::generateProgramNodeFromStatement("p", callNode));
}

TEST_CASE("Test Parser::parseExpr") {
    REQUIRE(Parser::parseExpr("1+y") == "(1+y)");
    REQUIRE(Parser::parseExpr("1+2*3") == "(1+(2*3))");
    REQUIRE(Parser::parseExpr("1*2+3") == "((1*2)+3)");
    REQUIRE(Parser::parseExpr("1+y*z+4*3") == "((1+(y*z))+(4*3))");
    REQUIRE(Parser::parseExpr("1+2+3-4") == "(((1+2)+3)-4)");
    REQUIRE(Parser::parseExpr("1*2%3/4") == "(((1*2)%3)/4)");
}

TEST_CASE("Test statement requires at least 2 tokens to parse") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure procedure { read");
    REQUIRE_THROWS_WITH(parser.parse(),
                        "Failed to parse statement: must have at least 2 tokens remaining");
}

TEST_CASE("Test assignment statements with keywords as assignee") {
    Parser parser = testhelpers::GenerateParserFromTokens("procedure procedure {"
                                                          "procedure = 1 + 1;"
                                                          "if = then + else;"
                                                          "then = else + while;"
                                                          "else = while + call;"
                                                          "while = call + print;"
                                                          "call = print + read;"
                                                          "print = read + procedure;"
                                                          "}");
    REQUIRE_NOTHROW(parser.parse());
}
} // namespace testparser
} // namespace backend
