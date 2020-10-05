#include "DesignExtractor.h"
#include "Logger.h"
#include "PKB.h"
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

TEST_CASE("Test getProcedureToCallees") {
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
    auto procedureToCallees = extractor::getProcedureToCallees(tNodeTypeToTNodes);

    // Every Procedure has an entry in the call graph
    REQUIRE(procedureToCallees.size() == 5);
    REQUIRE(procedureToCallees.find(p) != procedureToCallees.end());
    REQUIRE(procedureToCallees.find(q) != procedureToCallees.end());
    REQUIRE(procedureToCallees.find(r) != procedureToCallees.end());
    REQUIRE(procedureToCallees.find(s) != procedureToCallees.end());
    REQUIRE(procedureToCallees.find(t) != procedureToCallees.end());

    std::unordered_set<const TNode*> expectedCallees = { q };
    REQUIRE(procedureToCallees[p] == expectedCallees);

    expectedCallees = { r, s };
    REQUIRE(procedureToCallees[q] == expectedCallees);

    expectedCallees = { s };
    REQUIRE(procedureToCallees[r] == expectedCallees);

    expectedCallees = {};
    REQUIRE(procedureToCallees[s] == expectedCallees);
    REQUIRE(procedureToCallees[t] == expectedCallees);
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
    REQUIRE(usesMapping.size() == 9);
    for (auto& p : expectedUsesMappingForStatements) {
        if (p.second.empty()) {
            REQUIRE(usesMapping.find(p.first) == usesMapping.end());
        } else {
            REQUIRE(usesMapping.find(p.first) != usesMapping.end());
            REQUIRE(usesMapping[p.first] == p.second);
        }
    }

    // Regression test to ensure that "read y"; does not register "y" as a used variable.
    const char program2[] = "procedure AnotherProc {"
                            "while (x == 1) {"
                            "read y;"
                            "y = x + 2;"
                            "}"
                            "}";

    parser = testhelpers::GenerateParserFromTokens(program2);
    ast = TNode(parser.parse());
    tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    usesMapping = extractor::getUsesMapping(tNodeTypeToTNodes);

    std::unordered_set<std::string> expected = { "x" };
    REQUIRE(usesMapping[statementNumberToTNode[1]] == expected);
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
        REQUIRE(usesMapping[p.first] == p.second);
    }
}

TEST_CASE("Test getUsesMapping does not register empty sets") {
    const char program[] = "procedure Proc {"
                           "while (2 == 2) {"
                           "a = 1 + 2;" // nothing used
                           "read y;" // nothing used
                           "call AnotherProc;" // nothing used
                           "}"
                           "}"
                           "procedure AnotherProc {"
                           "read z;"
                           "b = 3+(4*5);"
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    auto usesMapping = extractor::getUsesMapping(tNodeTypeToTNodes);

    // Proc uses nothing
    REQUIRE(usesMapping.find(&ast.children[0]) == usesMapping.end());
    // While statement uses nothing
    REQUIRE(usesMapping.find(statementNumberToTNode[1]) == usesMapping.end());
    // Assign statement uses nothing
    REQUIRE(usesMapping.find(statementNumberToTNode[2]) == usesMapping.end());
    // Read statement uses nothing
    REQUIRE(usesMapping.find(statementNumberToTNode[3]) == usesMapping.end());
    // Call statement uses nothing
    REQUIRE(usesMapping.find(statementNumberToTNode[4]) == usesMapping.end());

    // AnotherProc uses nothing
    REQUIRE(usesMapping.find(&ast.children[1]) == usesMapping.end());
    // Read statement uses nothing
    REQUIRE(usesMapping.find(statementNumberToTNode[5]) == usesMapping.end());
    // Assign statement uses nothing
    REQUIRE(usesMapping.find(statementNumberToTNode[6]) == usesMapping.end());
}

TEST_CASE("Test getModifiesMapping with a single procedure") {
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
                           "read o;"
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    std::unordered_map<const TNode*, std::unordered_set<std::string>> expectedModifiesMappingForStatements;

    // The procedure used every used variable in the program.
    expectedModifiesMappingForStatements[&ast.children[0]] = { "b", "j", "d", "o", "e",
                                                               "f", "h", "k", "m" };

    // The first while loop, "while (a == 1) {...",
    expectedModifiesMappingForStatements[statementNumberToTNode[1]] = { "b", "d", "e", "f" };
    // "b = 1;" Modifies b
    expectedModifiesMappingForStatements[statementNumberToTNode[2]] = { "b" };
    // "while (c == 1) { d = 1; }" Modifies d
    expectedModifiesMappingForStatements[statementNumberToTNode[3]] = { "d" };
    expectedModifiesMappingForStatements[statementNumberToTNode[4]] = { "d" };
    // "if (e == 1) then { e = f; } else { f = 1; }" Modifies e,f
    expectedModifiesMappingForStatements[statementNumberToTNode[5]] = { "e", "f" };
    expectedModifiesMappingForStatements[statementNumberToTNode[6]] = { "e" };
    expectedModifiesMappingForStatements[statementNumberToTNode[7]] = { "f" };

    // "if (g == 1) then {..."
    expectedModifiesMappingForStatements[statementNumberToTNode[8]] = { "h", "j", "k" };
    // "h = 1;"
    expectedModifiesMappingForStatements[statementNumberToTNode[9]] = { "h" };
    // "while (i == 1) { j = 1; }"
    expectedModifiesMappingForStatements[statementNumberToTNode[10]] = { "j" };
    expectedModifiesMappingForStatements[statementNumberToTNode[11]] = { "j" };
    // "k = 1;"
    expectedModifiesMappingForStatements[statementNumberToTNode[12]] = { "k" };
    // "print l;"
    expectedModifiesMappingForStatements[statementNumberToTNode[13]] = {};
    // "m = 23 + n;"
    expectedModifiesMappingForStatements[statementNumberToTNode[14]] = { "m" };
    // "read o;"
    expectedModifiesMappingForStatements[statementNumberToTNode[15]] = { "o" };

    auto modifiesMapping = extractor::getModifiesMapping(tNodeTypeToTNodes);
    REQUIRE(modifiesMapping.size() == 15);
    for (auto& p : expectedModifiesMappingForStatements) {
        if (p.second.empty()) {
            REQUIRE(modifiesMapping.find(p.first) == modifiesMapping.end());
        } else {
            REQUIRE(modifiesMapping.find(p.first) != modifiesMapping.end());
            REQUIRE(modifiesMapping[p.first] == p.second);
        }
    }
}

TEST_CASE("Test getModifiesMapping with multiple procedures") {
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

    std::unordered_map<const TNode*, std::unordered_set<std::string>> expectedModifiesMappingForStatements;
    expectedModifiesMappingForStatements[s] = { "d" };
    // assign dog
    expectedModifiesMappingForStatements[statementNumberToTNode[10]] = { "d" };

    expectedModifiesMappingForStatements[r] = { "d", "c" };
    // if statement
    expectedModifiesMappingForStatements[statementNumberToTNode[4]] = { "c", "d" };
    // call s
    expectedModifiesMappingForStatements[statementNumberToTNode[5]] = { "d" };
    // assign cat
    expectedModifiesMappingForStatements[statementNumberToTNode[6]] = { "c" };

    expectedModifiesMappingForStatements[q] = { "a", "c", "d" };
    // call r
    expectedModifiesMappingForStatements[statementNumberToTNode[7]] = { "c", "d" };
    // call s
    expectedModifiesMappingForStatements[statementNumberToTNode[8]] = { "d" };
    // assign apple
    expectedModifiesMappingForStatements[statementNumberToTNode[9]] = { "a" };

    expectedModifiesMappingForStatements[p] = { "a", "b", "c", "d" };
    // while container
    expectedModifiesMappingForStatements[statementNumberToTNode[1]] = { "a", "c", "d" };
    // call q
    expectedModifiesMappingForStatements[statementNumberToTNode[2]] = { "a", "c", "d" };
    // assign ball
    expectedModifiesMappingForStatements[statementNumberToTNode[3]] = { "b" };

    auto ModifiesMapping = extractor::getModifiesMapping(tNodeTypeToTNodes);
    REQUIRE(ModifiesMapping.size() == 14);
    for (auto& p : expectedModifiesMappingForStatements) {
        REQUIRE(p.second == ModifiesMapping[p.first]);
    }
}

TEST_CASE("Test getModifiesMapping does not register empty sets") {
    const char program[] = "procedure Proc {"
                           "while (x == (y + z)) {" // nothing modified
                           "print a;" // nothing modified
                           "print b;" // nothing modified
                           "call AnotherProc;" // nothing modified
                           "}"
                           "}"
                           "procedure AnotherProc {"
                           "print c;"
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    auto modifiesMapping = extractor::getModifiesMapping(tNodeTypeToTNodes);

    // Proc uses nothing
    REQUIRE(modifiesMapping.find(&ast.children[0]) == modifiesMapping.end());
    // While statement uses nothing
    REQUIRE(modifiesMapping.find(statementNumberToTNode[1]) == modifiesMapping.end());
    // Print statement uses nothing
    REQUIRE(modifiesMapping.find(statementNumberToTNode[2]) == modifiesMapping.end());
    // Print statement uses nothing
    REQUIRE(modifiesMapping.find(statementNumberToTNode[3]) == modifiesMapping.end());
    // Call statement uses nothing
    REQUIRE(modifiesMapping.find(statementNumberToTNode[4]) == modifiesMapping.end());

    // AnotherProc uses nothing
    REQUIRE(modifiesMapping.find(&ast.children[0]) == modifiesMapping.end());
    // Print statement modifies nothing
    REQUIRE(modifiesMapping.find(statementNumberToTNode[5]) == modifiesMapping.end());
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
    STATEMENT_NUMBER_SET actual = extractor::getKeysInMap(input);

    STATEMENT_NUMBER_SET expected = { 1, 2, 3 };
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVisitedPathFromStart") {
    std::unordered_map<int, int> input = { { 1, 2 }, { 2, 3 }, { 3, 4 } };
    STATEMENT_NUMBER_SET actual = extractor::getVisitedPathFromStart(1, input);

    STATEMENT_NUMBER_SET expected = { 2, 3, 4 };
    REQUIRE(actual == expected);
}

TEST_CASE("Test getParentRelationship") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    std::unordered_map<int, STATEMENT_NUMBER_SET> actualParentChildren;
    std::unordered_map<int, int> actualChildParent;
    std::tie(actualChildParent, actualParentChildren) = extractor::getParentRelationship(ast);
    std::vector<std::pair<int, int>> actualChildParentVector(actualChildParent.begin(),
                                                             actualChildParent.end());
    std::vector<std::pair<int, STATEMENT_NUMBER_SET>> actualParentChildrenVector(
    actualParentChildren.begin(), actualParentChildren.end());

    std::vector<std::pair<int, STATEMENT_NUMBER_SET>> expectedParentChildren = { { 1, { 2 } },
                                                                                 { 3, { 4, 5 } } };
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

TEST_CASE("Test getStatementNumberToTNodeTypeMap") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"

                                        "while (y == 3) {"
                                        "gucci = 1;"
                                        "}"

                                        "if (!(armani == gucci)) then {"
                                        "read x;"
                                        "call y;"
                                        "} else {"
                                        "print z;"
                                        "}"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    std::unordered_map<int, TNodeType> result =
    extractor::getStatementNumberToTNodeTypeMap(statementNumberToTNode);

    std::unordered_map<int, TNodeType> expected = {
        { 1, While }, { 2, Assign }, { 3, TNodeType::IfElse },
        { 4, Read },  { 5, Call },   { 6, Print }
    };
    REQUIRE(result == expected);
}

TEST_CASE("isValidSimpleProgram: Two procedures with the same name is invalid") {
    const char program[] = "procedure p {x=1;} procedure p{y=1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    REQUIRE(extractor::isValidSimpleProgram(ast) == false);
}

TEST_CASE("isValidSimpleProgram: Call to non-existing procedures is invalid") {
    const char program[] = "procedure p {call q;}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    REQUIRE(extractor::isValidSimpleProgram(ast) == false);
}

TEST_CASE("isValidSimpleProgram: Recursive call is invalid") {
    const char program[] = "procedure p {call p;}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    REQUIRE(extractor::isValidSimpleProgram(ast) == false);
}

TEST_CASE("isValidSimpleProgram: Cyclic calls are invalid ") {
    const char program[] = "procedure p {call q;}"
                           "procedure q {call r;}"
                           "procedure r {call p;}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    REQUIRE(extractor::isValidSimpleProgram(ast) == false);
}

TEST_CASE("isValidSimpleProgram: Acyclic calls are valid") {
    const char program[] = "procedure p {call r;}"
                           "procedure q {call s;}"
                           "procedure r {call s;}"
                           "procedure s {x = 1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    REQUIRE(extractor::isValidSimpleProgram(ast) == true);
}

TEST_CASE("Test getNextRelationship double jump") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "  while (y == 3) {" // 1
                                        "    gucci = 1;" // 2
                                        "  }"
                                        "  if (!(armani == gucci)) then {" // 3
                                        "    read x;" // 4
                                        "    if (1+1 == 2) then {" // 5
                                        "      if (1+1 == 2) then {" // 6
                                        "        y = 1;" // 7
                                        "      } else {"
                                        "        print z;" // 8
                                        "      }"
                                        "    } else {"
                                        "      print z;" // 9
                                        "    }"
                                        "  } else {"
                                        "    if (1+1 == 2) then {" // 10
                                        "      y = 1;" // 11
                                        "    } else {"
                                        "      print z;" // 12
                                        "      y = 1;" // 13
                                        "    }"
                                        "  }"
                                        "  y = y + 1;" // 14
                                        "  y = 1;" // 15
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = {
        { 1, { 2, 3 } }, { 2, { 1 } },   { 3, { 4, 10 } }, { 4, { 5 } },  { 5, { 6, 9 } },
        { 6, { 7, 8 } }, { 7, { 14 } },  { 8, { 14 } },    { 9, { 14 } }, { 10, { 11, 12 } },
        { 11, { 14 } },  { 12, { 13 } }, { 13, { 14 } },   { 14, { 15 } }
    };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getNextRelationship if-else jumps to while") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "  while (y == 3) {"
                                        "    if (1==1) then {"
                                        "       y = 1;"
                                        "    } else {"
                                        "       y = 2;"
                                        "    }"
                                        "  }"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = {

        { 1, { 2 } }, { 2, { 3, 4 } }, { 3, { 1 } }, { 4, { 1 } }
    };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getNextRelationship if-else dead end") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "  if (1==1) then {"
                                        "     y = 1;"
                                        "  } else {"
                                        "     y = 2;"
                                        "  }"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = { { 1, { 2, 3 } } };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getNextRelationship while-loop in if-else") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "  if (1==1) then {" // 1
                                        "    while (1==1) {" // 2
                                        "      a = 1;" // 3
                                        "    }"
                                        "  } else {"
                                        "    while (1==1) {" // 4
                                        "      a = 1;" // 5
                                        "    }"
                                        "  }"
                                        "  a = 1;" // 6
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = {
        { 1, { 2, 4 } }, { 2, { 3, 6 } }, { 3, { 2 } }, { 4, { 5, 6 } }, { 5, { 4 } }
    };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getNextRelationship nested while-loop") {
    const char STRUCTURED_STATEMENT[] = "procedure a {         "
                                        "  while (1 == 1) {    " // 1
                                        "    while (1==1) {    " // 2
                                        "      a = 1;          " // 3
                                        "      while (1 == 1) {" // 4
                                        "        a = 1;        " // 5
                                        "        a = 2;        " // 6
                                        "      }"
                                        "    }"
                                        "    a = 1;            " // 7
                                        "  }"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = { { 1, { 2 } }, { 2, { 3, 7 } },
                                                                  { 3, { 4 } }, { 4, { 2, 5 } },
                                                                  { 5, { 6 } }, { 6, { 4 } },
                                                                  { 7, { 1 } } };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getNextRelationship two procedure") {
    const char STRUCTURED_STATEMENT[] = "procedure a {         "
                                        "  while (1 == 1) {    " // 1
                                        "    while (1==1) {    " // 2
                                        "      a = 1;          " // 3
                                        "      while (1 == 1) {" // 4
                                        "        a = 1;        " // 5
                                        "        a = 2;        " // 6
                                        "      }"
                                        "    }"
                                        "    a = 1;            " // 7
                                        "  }"
                                        "}"

                                        "procedure b {         "
                                        "  while (1 == 1) {    " // 8
                                        "    while (1==1) {    " // 9
                                        "      a = 1;          " // 10
                                        "      while (1 == 1) {" // 11
                                        "        a = 1;        " // 12
                                        "        a = 2;        " // 13
                                        "      }"
                                        "    }"
                                        "    a = 1;            " // 14
                                        "  }"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = {
        { 1, { 2 } },      { 2, { 3, 7 } }, { 3, { 4 } },   { 4, { 2, 5 } },   { 5, { 6 } },
        { 6, { 4 } },      { 7, { 1 } },    { 8, { 9 } },   { 9, { 10, 14 } }, { 10, { 11 } },
        { 11, { 9, 12 } }, { 12, { 13 } },  { 13, { 11 } }, { 14, { 8 } }
    };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getNextRelationship alternate ") {
    const char STRUCTURED_STATEMENT[] = "procedure a {            "
                                        "  if (1==1) then {       " // 1
                                        "    while (1==1) {       " // 2
                                        "      if (1==1) then {   " // 3
                                        "        while (1 == 1) { " // 4
                                        "          a = 1;         " // 5
                                        "        }"
                                        "      } else {"
                                        "        while (1 == 1) { " // 6
                                        "          a = 1;         " // 7
                                        "        }"
                                        "      }"
                                        "    }"
                                        "  } else {"
                                        "    a = 1;               " // 8
                                        "  }"
                                        "  a = 2;                 " // 9
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);
    std::unordered_map<int, std::unordered_set<int>> expected = { { 1, { 2, 8 } }, { 2, { 3, 9 } },
                                                                  { 3, { 4, 6 } }, { 4, { 5, 2 } },
                                                                  { 5, { 4 } },    { 6, { 7, 2 } },
                                                                  { 7, { 6 } },    { 8, { 9 } } };

    REQUIRE(actual == expected);
}

TEST_CASE("Test getPreviousRelationship") {
    const char STRUCTURED_STATEMENT[] = "procedure a {         "
                                        "  while (1 == 1) {    " // 1
                                        "    while (1==1) {    " // 2
                                        "      a = 1;          " // 3
                                        "      while (1 == 1) {" // 4
                                        "        a = 1;        " // 5
                                        "        a = 2;        " // 6
                                        "      }"
                                        "    }"
                                        "    a = 1;            " // 7
                                        "  }"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeTypeToTNodes = extractor::getTNodeTypeToTNodes(ast);
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    std::unordered_map<int, std::unordered_set<int>> nextRelationship =
    extractor::getNextRelationship(tNodeTypeToTNodes, tNodeToStatementNumber);

    std::unordered_map<int, std::unordered_set<int>> actual =
    extractor::getPreviousRelationship((nextRelationship));
    std::unordered_map<int, std::unordered_set<int>> expected = { { 1, { 7 } }, { 2, { 1, 4 } },
                                                                  { 3, { 2 } }, { 4, { 3, 6 } },
                                                                  { 5, { 4 } }, { 6, { 5 } },
                                                                  { 7, { 2 } } };

    REQUIRE(actual == expected);
}

} // namespace testextractor
} // namespace backend
