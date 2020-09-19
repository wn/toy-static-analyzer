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

    auto tNodeToStatementNumber = extractor::getTNodeTypeToTNodes(ast);
    for (auto k : tNodeToStatementNumber) {
        for (const TNode* i : k.second) {
            REQUIRE(i->type == k.first);
        }
    }
    REQUIRE(tNodeToStatementNumber[While].size() == 1);
    REQUIRE(tNodeToStatementNumber[Program].size() == 1);
    REQUIRE(tNodeToStatementNumber[Procedure].size() == 1);
    REQUIRE(tNodeToStatementNumber[StatementList].size() == 4);
    REQUIRE(tNodeToStatementNumber[Assign].size() == 4);
    REQUIRE(tNodeToStatementNumber[Not].size() == 1);
    REQUIRE(tNodeToStatementNumber[Equal].size() == 2);
    REQUIRE(tNodeToStatementNumber[Plus].size() == 1);
    REQUIRE(tNodeToStatementNumber[Variable].size() == 8);
    REQUIRE(tNodeToStatementNumber[Constant].size() == 5);
    REQUIRE(tNodeToStatementNumber[INVALID].size() == 0);
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
} // namespace testextractor
} // namespace backend
