#include "DesignExtractor.h"
#include "Logger.h"
#include "TestParserHelpers.h"
#include "catch.hpp"

namespace backend {
namespace testextractor {

const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"

                                    "while (y == 3) {"
                                    "gucci = 1;"
                                    "}"

                                    "if (!(armani == gucci)) then {"
                                    "armani = 7;"
                                    "} else {"
                                    "apple = 1;"
                                    "}"

                                    "some_var = 23 + another_var;"
                                    "}";

TEST_CASE("Test getTNodeToStatementNumber gets all statements") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    const TNode ast(parser.parse());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);

    REQUIRE(tNodeToStatementNumber.size() == 6);
}

TEST_CASE("Test getTNodeToStatementNumber copies TNodes properly") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    TNode* procedureStatements = &ast.children[0].children[0];
    REQUIRE(procedureStatements->type == TNodeType::StatementList);

    REQUIRE(procedureStatements->children.size() == 3);

    TNode* whileNodeInAst = &procedureStatements->children[0];
    REQUIRE(whileNodeInAst != nullptr);
    REQUIRE(TNodeType::While == TNodeType::While);
    REQUIRE(whileNodeInAst->type == TNodeType::While);
    const TNode* whileNodeInMap = tNodeToStatementNumber.find(whileNodeInAst)->first;
    REQUIRE(whileNodeInMap == whileNodeInAst);

    TNode& ifNodeInAst = procedureStatements->children[1];
    REQUIRE(ifNodeInAst.type == TNodeType::IfElse);
    const TNode* ifNodeInMap = tNodeToStatementNumber.find(&ifNodeInAst)->first;
    REQUIRE(ifNodeInMap == &ifNodeInAst);

    TNode& assignNodeInAst = procedureStatements->children[2];
    REQUIRE(assignNodeInAst.type == TNodeType::Assign);
    const TNode* assignNodeInMap = tNodeToStatementNumber.find(&assignNodeInAst)->first;
    REQUIRE(assignNodeInMap == &assignNodeInAst);
}

TEST_CASE("Test getStatementNumberToTNode creates a reverse map of getTNodeToStatementNumber") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    REQUIRE(tNodeToStatementNumber.size() == statementNumberToTNode.size());

    for (auto& pair : statementNumberToTNode) {
        int statementNumber = pair.first;
        const TNode* tNode = pair.second;

        REQUIRE(tNodeToStatementNumber.find(tNode) != tNodeToStatementNumber.end());
        REQUIRE(tNodeToStatementNumber[tNode] == statementNumber);
    }
}

TEST_CASE("Test getTNodeToStatementNumber maps TNodes to their correct statement numbers") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    TNode* statementList = &ast.children[0].children[0];
    REQUIRE(statementList->type == TNodeType::StatementList);

    REQUIRE(tNodeToStatementNumber.size() == 6);

    // While statement
    TNode& whileNodeInAst = statementList->children[0];
    REQUIRE(whileNodeInAst.type == TNodeType::While);
    REQUIRE(statementNumberToTNode[1] == &whileNodeInAst);
    TNode& whileNodeStatements = whileNodeInAst.children[1];
    REQUIRE(whileNodeStatements.type == TNodeType::StatementList);
    REQUIRE(statementNumberToTNode[2] == &whileNodeStatements.children[0]);

    // If-Then-Else statement
    TNode& ifNodeInAst = statementList->children[1];
    REQUIRE(ifNodeInAst.type == TNodeType::IfElse);
    REQUIRE(statementNumberToTNode[3] == &ifNodeInAst);
    // If -> Then
    TNode& ifNodeThenStatements = ifNodeInAst.children[1];
    REQUIRE(ifNodeThenStatements.type == TNodeType::StatementList);
    REQUIRE(statementNumberToTNode[4] == &ifNodeThenStatements.children[0]);
    // If -> Else
    TNode& ifNodeElseStatements = ifNodeInAst.children[2];
    REQUIRE(ifNodeElseStatements.type == TNodeType::StatementList);
    REQUIRE(statementNumberToTNode[5] == &ifNodeElseStatements.children[0]);

    REQUIRE(statementNumberToTNode[6]->type == TNodeType::Assign);
    REQUIRE(statementNumberToTNode[6] == &statementList->children[2]);
}

TEST_CASE("Test getTNodeTypeToTNodes maps TNode to TNodeType correctly") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    for (auto k : tNodeTypeToTNodes) {
        for (const TNode* i : k.second) {
            REQUIRE(i->type == k.first);
        }
    }
    REQUIRE(tNodeTypeToTNodes[While].size() == 1);
    REQUIRE(tNodeTypeToTNodes[Program].size() == 1);
    REQUIRE(tNodeTypeToTNodes[Procedure].size() == 1);
    REQUIRE(tNodeTypeToTNodes[StatementList].size() == 4);
    REQUIRE(tNodeTypeToTNodes[Assign].size() == 4);
    REQUIRE(tNodeTypeToTNodes[Not].size() == 1);
    REQUIRE(tNodeTypeToTNodes[Equal].size() == 2);
    REQUIRE(tNodeTypeToTNodes[Plus].size() == 1);
    REQUIRE(tNodeTypeToTNodes[Variable].size() == 8);
    REQUIRE(tNodeTypeToTNodes[Constant].size() == 5);
    REQUIRE(tNodeTypeToTNodes[INVALID].size() == 0);
}

TEST_CASE("Test getProcedureFromProcedureName maps properly") {
    std::string MULTIPLE_PROCEDURES = "procedure p { x = 1;} procedure q {y = 2;}";
    Parser parser = testhelpers::GenerateParserFromTokens(MULTIPLE_PROCEDURES);
    TNode ast(parser.parse());
    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    REQUIRE(*extractor::getProcedureFromProcedureName("p", tNodeTypeToTNodes) == ast.children[0]);
    REQUIRE(*extractor::getProcedureFromProcedureName("q", tNodeTypeToTNodes) == ast.children[1]);
}

TEST_CASE("Test getProcedureToCallers") {
    const char program[] = "procedure p{call q;}"
                           "procedure q{call r; call s;}"
                           "procedure r{call s;}"
                           "procedure s{x = 2;}"
                           "procedure t{y = 1;}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    TNode* p = &ast.children[0];
    TNode* q = &ast.children[1];
    TNode* r = &ast.children[2];
    TNode* s = &ast.children[3];
    TNode* t = &ast.children[4];

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    REQUIRE(tNodeTypeToTNodes[TNodeType::Procedure].size() == 5);
    auto procedureToCallers = extractor::getProcedureToCallers(tNodeTypeToTNodes);

    // Only q, r and s are ever called.
    REQUIRE(procedureToCallers.size() == 3);

    // s is called by r and q
    REQUIRE(procedureToCallers.find(s) != procedureToCallers.end());
    auto callers = procedureToCallers.find(s)->second;
    REQUIRE(callers.size() == 2);
    REQUIRE(std::find(callers.begin(), callers.end(), r) != callers.end());
    REQUIRE(std::find(callers.begin(), callers.end(), q) != callers.end());

    // r is called by q
    REQUIRE(procedureToCallers.find(r) != procedureToCallers.end());
    callers = procedureToCallers.find(r)->second;
    REQUIRE(callers.size() == 1);
    REQUIRE(std::find(callers.begin(), callers.end(), q) != callers.end());

    // q is called by p
    REQUIRE(procedureToCallers.find(q) != procedureToCallers.end());
    callers = procedureToCallers.find(q)->second;
    REQUIRE(callers.size() == 1);
    REQUIRE(std::find(callers.begin(), callers.end(), p) != callers.end());
}


TEST_CASE("Test getTopologicalOrderingOfCalledByGraph") {
    // We shuffle the procedures to ensure that their order of appearance in the program
    // does not affect the topological order of their calls.
    const char program[] = "procedure p{call q;}"
                           "procedure s{x = 2;}"
                           "procedure r{call s;}"
                           "procedure q{call r; call s;}"
                           "procedure t{y = 1;}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    const TNode* p = &ast.children[0];
    const TNode* q = &ast.children[3];
    const TNode* r = &ast.children[2];
    const TNode* s = &ast.children[1];
    const TNode* t = &ast.children[4];

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    auto procedureReverseTopologicalOrder = extractor::getTopologicalOrderingOfCalledByGraph(tNodeTypeToTNodes);
    std::vector<const TNode*> expectedOrdering = {
        s, // s is called by r, does not make any calls.
        r, // r calls s
        q, // q calls r and s
        p, // p calls q, and indirectly calls q,r,s
        t // We can place t anywhere in the topological call order as it never calls/gets called.
    };
    REQUIRE(procedureReverseTopologicalOrder == expectedOrdering);
}

TEST_CASE("Test getUsesMapping with a single procedure") {
    const char program[] = "procedure MySpecialProc {"
                           "while (a == 1) {"
                           "b = 1;"
                           "while (c == 1) { d = 1; }"
                           "if (e == 1) then { e = f; } else { f = 1; }"
                           "}"
                           "if (g == 1) then {"
                           "h = 1;"
                           "while (i == 1) { j = 1; }"
                           "} else {"
                           "k = 1;"
                           "print l;"
                           "}"
                           "m = 23 + n;"
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    std::unordered_map<const TNode*, std::unordered_set<std::string>> expectedUsesMappingForStatements;

    // The procedure used every used variable in the program.
    expectedUsesMappingForStatements[&ast.children[0]] = { "a", "c", "e", "f", "g", "i", "l", "n" };

    // The first while loop, "while (a == 1) {...", uses a and everything it's children uses.
    expectedUsesMappingForStatements[statementNumberToTNode[1]] = { "a", "c", "e", "f" };
    // "b = 1;" uses nothing
    expectedUsesMappingForStatements[statementNumberToTNode[2]] = {};
    // "while (c == 1) { d = 1; }" uses c
    expectedUsesMappingForStatements[statementNumberToTNode[3]] = { "c" };
    expectedUsesMappingForStatements[statementNumberToTNode[4]] = {};
    // "if (e == 1) then { e = f; } else { f = 1; }" uses e,f
    expectedUsesMappingForStatements[statementNumberToTNode[5]] = { "e", "f" };
    expectedUsesMappingForStatements[statementNumberToTNode[6]] = { "f" };
    expectedUsesMappingForStatements[statementNumberToTNode[7]] = {};

    // "if (g == 1) then {..."
    expectedUsesMappingForStatements[statementNumberToTNode[8]] = { "g", "i", "l" };
    // "h = 1;"
    expectedUsesMappingForStatements[statementNumberToTNode[9]] = {};
    // "while (i == 1) { j = 1; }"
    expectedUsesMappingForStatements[statementNumberToTNode[10]] = { "i" };
    expectedUsesMappingForStatements[statementNumberToTNode[11]] = {};
    // "k = 1;"
    expectedUsesMappingForStatements[statementNumberToTNode[12]] = {};
    // "print l;"
    expectedUsesMappingForStatements[statementNumberToTNode[13]] = { "l" };
    // "m = 23 + n;"
    expectedUsesMappingForStatements[statementNumberToTNode[14]] = { "n" };

    auto usesMapping = extractor::getUsesMapping(tNodeTypeToTNodes);
    REQUIRE(usesMapping.size() == 15);
    for (auto& p : expectedUsesMappingForStatements) {
        REQUIRE(usesMapping.count(p.first));
        REQUIRE(p.second == usesMapping[p.first]);
    }
}

TEST_CASE("Test getUsesMapping with multiple procedures") {
    const char program[] = "procedure p { while (1 == 1) { call q; } b = ball;}"
                           "procedure r { if (1 == 1) then {call s;} else {c = cat;} }"
                           "procedure q { call r; call s; a = apple; }"
                           "procedure s { d = dog; }";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    TNode* p = &ast.children[0];
    TNode* r = &ast.children[1];
    TNode* q = &ast.children[2];
    TNode* s = &ast.children[3];

    std::unordered_map<const TNode*, std::unordered_set<std::string>> expectedUsesMappingForStatements;
    expectedUsesMappingForStatements[s] = { "dog" };
    // assign dog
    expectedUsesMappingForStatements[statementNumberToTNode[10]] = { "dog" };

    expectedUsesMappingForStatements[r] = { "cat", "dog" };
    // if statement
    expectedUsesMappingForStatements[statementNumberToTNode[4]] = { "cat", "dog" };
    // call s
    expectedUsesMappingForStatements[statementNumberToTNode[5]] = { "dog" };
    // assign cat
    expectedUsesMappingForStatements[statementNumberToTNode[6]] = { "cat" };

    expectedUsesMappingForStatements[q] = { "apple", "cat", "dog" };
    // call r
    expectedUsesMappingForStatements[statementNumberToTNode[7]] = { "cat", "dog" };
    // call s
    expectedUsesMappingForStatements[statementNumberToTNode[8]] = { "dog" };
    // assign apple
    expectedUsesMappingForStatements[statementNumberToTNode[9]] = { "apple" };

    expectedUsesMappingForStatements[p] = { "apple", "ball", "cat", "dog" };
    // while container
    expectedUsesMappingForStatements[statementNumberToTNode[1]] = { "apple", "cat", "dog" };
    // call q
    expectedUsesMappingForStatements[statementNumberToTNode[2]] = { "apple", "cat", "dog" };
    // assign ball
    expectedUsesMappingForStatements[statementNumberToTNode[3]] = { "ball" };

    auto usesMapping = extractor::getUsesMapping(tNodeTypeToTNodes);
    REQUIRE(usesMapping.size() == 14);
    for (auto& p : expectedUsesMappingForStatements) {
        REQUIRE(usesMapping.count(p.first));
        REQUIRE(p.second == usesMapping[p.first]);
    }
}

TEST_CASE("Test getFollowRelationship") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    std::unordered_map<int, int> actualFollowFollowed;
    std::unordered_map<int, int> actualFollowedFollow;
    std::tie(actualFollowFollowed, actualFollowedFollow) = extractor::getFollowRelationship(ast);
    std::vector<std::pair<int, int>> actualFollowFollowedVector = { { 3, 1 }, { 6, 3 } };
    std::vector<std::pair<int, int>> actualFollowedFollowVector = { { 1, 3 }, { 3, 6 } };
    std::vector<std::pair<int, int>> expectedFollowFollowedVector = { { 3, 1 }, { 6, 3 } };
    std::vector<std::pair<int, int>> expectedFollowedFollowVector = { { 1, 3 }, { 3, 6 } };

    REQUIRE(actualFollowFollowedVector == expectedFollowFollowedVector);
    REQUIRE(actualFollowedFollowVector == expectedFollowedFollowVector);
}

TEST_CASE("Test getKeysInMap") {
    std::unordered_map<int, int> input = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
    std::vector<int> actual = extractor::getKeysInMap(input);

    std::vector<int> expected = { 1, 2, 3 };
    std::sort(actual.begin(), actual.end());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVisitedPathFromStart") {
    std::unordered_map<int, int> input = { { 1, 2 }, { 2, 3 }, { 3, 4 } };
    std::vector<int> actual = extractor::getVisitedPathFromStart(1, input);

    std::vector<int> expected = { 2, 3, 4 };
    std::sort(actual.begin(), actual.end());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getParentRelationship") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    std::unordered_map<int, std::vector<int>> actualParentChildren;
    std::unordered_map<int, int> actualChildParent;
    std::tie(actualChildParent, actualParentChildren) = extractor::getParentRelationship(ast);
    std::vector<std::pair<int, int>> actualChildParentVector(actualChildParent.begin(),
                                                             actualChildParent.end());
    std::vector<std::pair<int, std::vector<int>>> actualParentChildrenVector(actualParentChildren.begin(),
                                                                             actualParentChildren.end());

    std::vector<std::pair<int, std::vector<int>>> expectedParentChildren = { { 1, { 2 } }, { 3, { 4, 5 } } };
    std::vector<std::pair<int, int>> expectedChildParent = { { 2, 1 }, { 4, 3 }, { 5, 3 } };

    REQUIRE(actualParentChildrenVector == expectedParentChildren);
    REQUIRE(actualChildParentVector == expectedChildParent);
}

TEST_CASE("Test getPatternsMap multiple assigns") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    std::vector<const TNode*> assignTNodes = extractor::getTNodeTypeToTNodes(ast)[Assign];
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto result = extractor::getPatternsMap(assignTNodes, tNodeToStatementNumber);

    std::vector<std::tuple<std::string, int, bool>> result_c7 = { { "armani", 4, false } };
    REQUIRE(result["7"] == result_c7);
    std::vector<std::tuple<std::string, int, bool>> result_c1 = { { "apple", 5, false }, { "gucci", 2, false } };
    REQUIRE(result["1"] == result_c1);
    std::vector<std::tuple<std::string, int, bool>> result_23_plus_another_var = { { "some_var", 6, false } };
    REQUIRE(result["(23+another_var)"] == result_23_plus_another_var);
    std::vector<std::tuple<std::string, int, bool>> result_another_var = { { "some_var", 6, true } };
    REQUIRE(result["another_var"] == result_another_var);
    std::vector<std::tuple<std::string, int, bool>> result_23 = { { "some_var", 6, true } };
    REQUIRE(result["23"] == result_23);
}

TEST_CASE("Test getPatternsMap check precedence 1") {
    const char EXPR_STMT_PROG[] = "procedure p {"
                                  "x = 1 + 2 * 3;"
                                  "}";
    Parser parser = testhelpers::GenerateParserFromTokens(EXPR_STMT_PROG);
    TNode ast(parser.parse());

    std::vector<const TNode*> assignTNodes = extractor::getTNodeTypeToTNodes(ast)[Assign];
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto result = extractor::getPatternsMap(assignTNodes, tNodeToStatementNumber);

    std::vector<std::tuple<std::string, int, bool>> result_two_plus_three = { { "x", 1, true } };
    REQUIRE(result["(2*3)"] == result_two_plus_three);
    std::vector<std::tuple<std::string, int, bool>> result_three = { { "x", 1, true } };
    REQUIRE(result["3"] == result_three);
    std::vector<std::tuple<std::string, int, bool>> result_two = { { "x", 1, true } };
    REQUIRE(result["2"] == result_two);
    std::vector<std::tuple<std::string, int, bool>> result_expr = { { "x", 1, false } };
    REQUIRE(result["(1+(2*3))"] == result_expr);
    std::vector<std::tuple<std::string, int, bool>> result_one = { { "x", 1, true } };
    REQUIRE(result["1"] == result_one);
}

TEST_CASE("Test getPatternsMap check precedence 2") {
    const char EXPR_STMT_PROG[] = "procedure p {"
                                  "x = (1 + 2) * 3;"
                                  "}";
    Parser parser = testhelpers::GenerateParserFromTokens(EXPR_STMT_PROG);
    TNode ast(parser.parse());

    std::vector<const TNode*> assignTNodes = extractor::getTNodeTypeToTNodes(ast)[Assign];
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto result = extractor::getPatternsMap(assignTNodes, tNodeToStatementNumber);

    std::vector<std::tuple<std::string, int, bool>> result_1_plus_2 = { { "x", 1, true } };
    REQUIRE(result["(1+2)"] == result_1_plus_2);
    std::vector<std::tuple<std::string, int, bool>> result_three = { { "x", 1, true } };
    REQUIRE(result["3"] == result_three);
    std::vector<std::tuple<std::string, int, bool>> result_two = { { "x", 1, true } };
    REQUIRE(result["2"] == result_two);
    std::vector<std::tuple<std::string, int, bool>> result_expr = { { "x", 1, false } };
    REQUIRE(result["((1+2)*3)"] == result_expr);
    std::vector<std::tuple<std::string, int, bool>> result_one = { { "x", 1, true } };
    REQUIRE(result["1"] == result_one);
}

TEST_CASE("Test getPatternsMap check precedence complicated") {
    const char EXPR_STMT_PROG[] = "procedure p {"
                                  "x = (1 + 2) * x + y / (z - 3);"
                                  "}";
    Parser parser = testhelpers::GenerateParserFromTokens(EXPR_STMT_PROG);
    TNode ast(parser.parse());

    std::vector<const TNode*> assignTNodes = extractor::getTNodeTypeToTNodes(ast)[Assign];
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto result = extractor::getPatternsMap(assignTNodes, tNodeToStatementNumber);

    std::vector<std::tuple<std::string, int, bool>> result_three = { { "x", 1, true } };
    REQUIRE(result["3"] == result_three);
    std::vector<std::tuple<std::string, int, bool>> result_two = { { "x", 1, true } };
    REQUIRE(result["2"] == result_two);
    std::vector<std::tuple<std::string, int, bool>> result_one = { { "x", 1, true } };
    REQUIRE(result["1"] == result_one);
    std::vector<std::tuple<std::string, int, bool>> result_x = { { "x", 1, true } };
    REQUIRE(result["x"] == result_x);
    std::vector<std::tuple<std::string, int, bool>> result_y = { { "x", 1, true } };
    REQUIRE(result["y"] == result_y);
    std::vector<std::tuple<std::string, int, bool>> result_z = { { "x", 1, true } };
    REQUIRE(result["z"] == result_z);

    std::vector<std::tuple<std::string, int, bool>> result_y_div_z_minus_three = { { "x", 1, true } };
    REQUIRE(result["(y/(z-3))"] == result_y_div_z_minus_three);
    std::vector<std::tuple<std::string, int, bool>> result_z_minus_three = { { "x", 1, true } };
    REQUIRE(result["(z-3)"] == result_z_minus_three);

    std::vector<std::tuple<std::string, int, bool>> result_one_plus_two_times_x = { { "x", 1, true } };
    REQUIRE(result["((1+2)*x)"] == result_one_plus_two_times_x);
    std::vector<std::tuple<std::string, int, bool>> result_one_plus_two = { { "x", 1, true } };
    REQUIRE(result["(1+2)"] == result_one_plus_two);

    std::vector<std::tuple<std::string, int, bool>> result_expr = { { "x", 1, false } };
    REQUIRE(result["(((1+2)*x)+(y/(z-3)))"] == result_expr);
}
} // namespace testextractor
} // namespace backend
