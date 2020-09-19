#include "Logger.h"
#include "PKBImplementation.h"
#include "TestParserHelpers.h"
#include "catch.hpp"

#include <algorithm>

namespace backend {
namespace testpkb {

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

TEST_CASE("Test getStatementsFollowedBy") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = {};
    REQUIRE(pkb.getStatementsFollowedBy(1) == expected);

    std::vector<int> expected2 = {};
    REQUIRE(pkb.getStatementsFollowedBy(2) == expected2);

    std::vector<int> expected3 = { 1 };
    REQUIRE(pkb.getStatementsFollowedBy(3) == expected3);

    std::vector<int> expected4 = {};
    REQUIRE(pkb.getStatementsFollowedBy(4) == expected4);

    std::vector<int> expected5 = {};
    REQUIRE(pkb.getStatementsFollowedBy(5) == expected5);

    std::vector<int> expected6 = { 3, 1 };
    REQUIRE(pkb.getStatementsFollowedBy(6) == expected6);
}

TEST_CASE("Test getStatementsThatFollows") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = { 3, 6 };
    std::vector<int> actual = { 3, 6 };
    REQUIRE(actual == expected);

    std::vector<int> expected2 = {};
    REQUIRE(pkb.getStatementsThatFollows(2) == expected2);

    std::vector<int> expected3 = { 6 };
    REQUIRE(pkb.getStatementsThatFollows(3) == expected3);

    std::vector<int> expected4 = {};
    REQUIRE(pkb.getStatementsThatFollows(4) == expected4);

    std::vector<int> expected5 = {};
    REQUIRE(pkb.getStatementsThatFollows(5) == expected5);

    std::vector<int> expected6 = {};
    REQUIRE(pkb.getStatementsThatFollows(6) == expected6);
}

TEST_CASE("Test getAllStatementsThatFollows") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = { 3, 6 };
    std::vector<int> actual = pkb.getAllStatementsThatFollows();
    std::sort(actual.begin(), actual.end());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAllStatementsThatAreFollowed") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = { 1, 3 };
    std::vector<int> actual = pkb.getAllStatementsThatAreFollowed();
    std::sort(actual.begin(), actual.end());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAncestors") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = {};
    REQUIRE(pkb.getAncestors(1) == expected);

    std::vector<int> expected2 = { 1 };
    REQUIRE(pkb.getAncestors(2) == expected2);

    std::vector<int> expected3 = {};
    REQUIRE(pkb.getAncestors(3) == expected3);

    std::vector<int> expected4 = { 3 };
    REQUIRE(pkb.getAncestors(4) == expected4);

    std::vector<int> expected5 = { 3 };
    REQUIRE(pkb.getAncestors(5) == expected5);

    std::vector<int> expected6 = {};
    REQUIRE(pkb.getAncestors(6) == expected6);
}

TEST_CASE("Test getDescendants") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = { 2 };
    REQUIRE(pkb.getDescendants(1) == expected);

    std::vector<int> expected2 = {};
    REQUIRE(pkb.getDescendants(2) == expected2);

    std::vector<int> expected3 = { 4, 5 };
    std::vector<int> actual3 = pkb.getDescendants(3);
    sort(actual3.begin(), actual3.end());
    REQUIRE(actual3 == expected3);

    std::vector<int> expected4 = {};
    REQUIRE(pkb.getDescendants(4) == expected4);

    std::vector<int> expected5 = {};
    REQUIRE(pkb.getDescendants(5) == expected5);

    std::vector<int> expected6 = {};
    REQUIRE(pkb.getDescendants(6) == expected6);
}

TEST_CASE("Test getStatementsThatHaveAncestors") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::vector<int> expected = { 2, 4, 5 };
    std::vector<int> actual = pkb.getStatementsThatHaveAncestors();
    std::sort(actual.begin(), actual.end());

    REQUIRE(expected == actual);
}

TEST_CASE("Test getStatementsThatHaveDescendants") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::vector<int> expected = { 1, 3 };
    std::vector<int> actual = pkb.getStatementsThatHaveDescendants();
    std::sort(actual.begin(), actual.end());

    REQUIRE(expected == actual);
}
} // namespace testpkb
} // namespace backend
