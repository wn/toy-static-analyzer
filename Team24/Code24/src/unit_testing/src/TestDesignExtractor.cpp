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
    TNode ast(parser.parse());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);

    REQUIRE(tNodeToStatementNumber.size() == 6);
}

TEST_CASE("Test getTNodeToStatementNumber copies TNodes properly") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    const TNode& ast(parser.parse());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    TNode procedureStatements = ast.children[0].children[0];
    REQUIRE(procedureStatements.type == TNodeType::StatementList);

    REQUIRE(procedureStatements.children.size() == 3);

    const TNode& whileNodeInAst = procedureStatements.children[0];
    REQUIRE(whileNodeInAst.type == TNodeType::While);
    const TNode& whileNodeInMap = tNodeToStatementNumber.find(whileNodeInAst)->first;
    REQUIRE(whileNodeInMap == whileNodeInAst);

    const TNode& ifNodeInAst = procedureStatements.children[1];
    REQUIRE(ifNodeInAst.type == TNodeType::IfElse);
    const TNode& ifNodeInMap = tNodeToStatementNumber.find(ifNodeInAst)->first;
    REQUIRE(ifNodeInMap == ifNodeInMap);

    const TNode& assignNodeInAst = procedureStatements.children[2];
    REQUIRE(assignNodeInAst.type == TNodeType::Assign);
    const TNode& assignNodeInMap = tNodeToStatementNumber.find(assignNodeInAst)->first;
    REQUIRE(assignNodeInAst == assignNodeInMap);
}

TEST_CASE("Test getStatementNumberToTNode creates a reverse map of getTNodeToStatementNumber") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());

    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    REQUIRE(tNodeToStatementNumber.size() == statementNumberToTNode.size());

    for (auto& pair : statementNumberToTNode) {
        int statementNumber = pair.first;
        TNode tNode = pair.second;

        REQUIRE(tNodeToStatementNumber.find(tNode) != tNodeToStatementNumber.end());
        REQUIRE(tNodeToStatementNumber[tNode] == statementNumber);
    }
}

TEST_CASE("Test getTNodeToStatementNumber maps TNodes to their correct statement numbers") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    TNode statementList = ast.children[0].children[0];
    REQUIRE(statementList.type == TNodeType::StatementList);

    REQUIRE(tNodeToStatementNumber.size() == 6);

    // While statement
    TNode whileNodeInAst = statementList.children[0];
    REQUIRE(whileNodeInAst.type == TNodeType::While);
    REQUIRE(statementNumberToTNode[1] == whileNodeInAst);
    TNode whileNodeStatements = whileNodeInAst.children[1];
    REQUIRE(whileNodeStatements.type == TNodeType::StatementList);
    REQUIRE(statementNumberToTNode[2] == whileNodeStatements.children[0]);

    // If-Then-Else statement
    TNode ifNodeInAst = statementList.children[1];
    REQUIRE(ifNodeInAst.type == TNodeType::IfElse);
    REQUIRE(statementNumberToTNode[3] == ifNodeInAst);
    // If -> Then
    TNode ifNodeThenStatements = ifNodeInAst.children[1];
    REQUIRE(ifNodeThenStatements.type == TNodeType::StatementList);
    REQUIRE(statementNumberToTNode[4] == ifNodeThenStatements.children[0]);
    // If -> Else
    TNode ifNodeElseStatements = ifNodeInAst.children[2];
    REQUIRE(ifNodeElseStatements.type == TNodeType::StatementList);
    REQUIRE(statementNumberToTNode[5] == ifNodeElseStatements.children[0]);

    REQUIRE(statementNumberToTNode[6].type == TNodeType::Assign);
    REQUIRE(statementNumberToTNode[6] == statementList.children[2]);
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

} // namespace testextractor
} // namespace backend
