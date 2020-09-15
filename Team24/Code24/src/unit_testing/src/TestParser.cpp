#include "Logger.h"
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
    procNode.name = "p";
    procNode.addChild(stmtNode);

    TNode progNode(TNodeType::Program, -1);
    progNode.addChild(procNode);
    return progNode;
}

// create an assign TNode for `y=y+1`.
TNode generateMockAssignNode() {
    TNode y(Variable, 1);
    y.name = "y";

    TNode constNode(Constant, 1);
    constNode.constant = 1;

    TNode plusNode(Plus);
    plusNode.addChild(y);
    plusNode.addChild(constNode);

    TNode assignNode(Assign, 1);
    assignNode.addChild(y);
    assignNode.addChild(plusNode);

    return assignNode;
}

// Procedure node name is p, statement within the body is `y=y+1`.
TNode generateProgramNodeFromCondition(const TNode& node) {
    TNode stmt = generateMockAssignNode();

    TNode innerStmtNode(TNodeType::StatementList, 1);
    innerStmtNode.addChild(stmt);

    TNode whileNode(TNodeType::While, 1);
    whileNode.addChild(node); // condition
    whileNode.addChild(innerStmtNode); // add dummy assign tnode to while-loop
    TNode stmtNode(TNodeType::StatementList, 1);
    stmtNode.addChild(whileNode);

    TNode procNode(TNodeType::Procedure, 1);
    procNode.name = "p";
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

    parser = GenerateParserFromTokens("procedure p{ if () then {x=2;} else {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect condition, none defined");

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

    parser = GenerateParserFromTokens("procedure p{ while () {x=2;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect condition, none defined");

    // empty statement list
    parser = GenerateParserFromTokens("procedure p{ while (xoxo == 1) {}}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect NAME, got RBRACE");
}

TEST_CASE("Test Condition with <= math operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z <= 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with >= math operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z >= 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(GreaterThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with > math operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z > 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(Greater, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with < math operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z < 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(Lesser, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with == math operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z == 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(Equal, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with != math operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z != 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(NotEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with AND operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while ((x < 1) && (y < 1)){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = 1;
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = 1;
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(And, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with OR operator") {
    Parser parser = GenerateParserFromTokens("procedure p{while ((x < 1) || (y < 1)){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = 1;
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = 1;
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(Or, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test Condition with NOT operator") {
    Parser parser =
    GenerateParserFromTokens("procedure p{while (!((x < 1) || (y < 1))){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = 1;
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = 1;
    TNode rCondLess(Lesser, 1);
    rCondLess.addChild(y);
    rCondLess.addChild(oneRNode);

    TNode condNode(Or, 1);
    condNode.addChild(lCondLess);
    condNode.addChild(rCondLess);

    TNode notNode(Not, 1);
    notNode.addChild(condNode);

    require(result == generateProgramNodeFromCondition(notNode));
}

TEST_CASE("Test Condition with Triple operator") {
    Parser parser = GenerateParserFromTokens(
    "procedure p{while (!(((x < 1) || (y < 1)) && ((x < 1) || (y < 1)))){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";
    TNode oneLNode(Constant, 1);
    oneLNode.constant = 1;
    TNode lCondLess(Lesser, 1);
    lCondLess.addChild(x);
    lCondLess.addChild(oneLNode);

    TNode y(Variable, 1);
    y.name = "y";
    TNode oneRNode(Constant, 1);
    oneRNode.constant = 1;
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

    require(result == generateProgramNodeFromCondition(notNode));
}

TEST_CASE("Test expr associativity + before *") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x + y * z <= 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSPlus);
    condNode.addChild(lCondRHSConst);

    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr associativity * before +") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x * y + z <= 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << generateProgramNodeFromCondition(condNode).toString() << std::endl;
    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr associativity * before *") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x * y * z <= 1){y = y + 1;}}");
    TNode result = parser.parse();


    // We expect our node to look like:
    //         *
    //        /  \
    //       x    *
    //           /  \
    //          y    z

    TNode x(Variable, 1);
    x.name = "x";
    TNode y(Variable, 1);
    y.name = "y";
    TNode z(Variable, 1);
    z.name = "z";

    TNode lCondLHSMultiply2(Multiply);
    lCondLHSMultiply2.addChild(y);
    lCondLHSMultiply2.addChild(z);

    TNode lCondLHSMultiply(Multiply);
    lCondLHSMultiply.addChild(x);
    lCondLHSMultiply.addChild(lCondLHSMultiply2);

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(lCondLHSMultiply);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = 1;
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << generateProgramNodeFromCondition(condNode).toString() << std::endl;
    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr associativity * before + with brackets") {
    Parser parser = GenerateParserFromTokens("procedure p{while (x * (y + z) <= 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << generateProgramNodeFromCondition(condNode).toString() << std::endl;
    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr associativity + before * with brackets") {
    Parser parser = GenerateParserFromTokens("procedure p{while ((x + y) * z <= 1){y = y + 1;}}");
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
    lCondRHSConst.constant = 1;
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << generateProgramNodeFromCondition(condNode).toString() << std::endl;
    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test expr useless brackets") {
    Parser parser = GenerateParserFromTokens("procedure p{while ((x) <= (1)){y = y + 1;}}");
    TNode result = parser.parse();

    TNode x(Variable, 1);
    x.name = "x";

    TNode condNode(LesserThanOrEqual, 1);
    condNode.addChild(x);
    TNode lCondRHSConst(Constant, 1);
    lCondRHSConst.constant = 1;
    condNode.addChild(lCondRHSConst);

    std::cout << result.toString() << std::endl;
    std::cout << generateProgramNodeFromCondition(condNode).toString() << std::endl;
    require(result == generateProgramNodeFromCondition(condNode));
}

TEST_CASE("Test condition, throw properly") {
    Parser parser = GenerateParserFromTokens("procedure p{while (+++){y = y + 1;}}");
    REQUIRE_THROWS_WITH(parser.parse(), "Failed to parse conditions");
}

TEST_CASE("Test parseAssign") {
    // Note: since expr has been tested in parseCondition, we did not write extensive test for
    // the RHS of assign.
    Parser parser = GenerateParserFromTokens("procedure p{x = y + 1;}");
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
    constNode.constant = 1;

    TNode plusNode(Plus);
    plusNode.addChild(y);
    plusNode.addChild(constNode);

    TNode assignNode(Assign, 1);
    assignNode.addChild(x);
    assignNode.addChild(plusNode);

    require(result == generateProgramNodeFromStatement("p", assignNode));
}

TEST_CASE("Test parseAssign wrong LHS") {
    Parser parser = GenerateParserFromTokens("procedure p{x + y = y + 1;}");
    REQUIRE_THROWS_WITH(parser.parse(), "expect SINGLE_EQ, got PLUS");
}
} // namespace testparser
} // namespace backend
