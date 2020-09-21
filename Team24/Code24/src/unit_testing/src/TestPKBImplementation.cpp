#include "Logger.h"
#include "PKBImplementation.h"
#include "TestParserHelpers.h"
#include "catch.hpp"

#include <algorithm>
#include <unordered_map>

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

const char USES_PROGRAM[] = "procedure caller {"
                            "while (y == 3) {" // 1
                            "gucci = 1;" // 2
                            "}"

                            "if (!(armani == gucci)) then {" // 3
                            "armani = 7;" // 4
                            "} else {"
                            "apple = 1;" // 5
                            "call callee;" // 6
                            "}"
                            "some_var = 23 + another_var;" //  7
                            "}"
                            "procedure callee {"
                            "microsoft = apple;" //  8
                            "google = microsoft;" //  9
                            "while(nerf == google) {" //  10
                            "nerf = roblox;" // 11
                            "}"
                            "}";

TEST_CASE("Test getStatementsThatUse") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_LIST> testCases = {
        { "another_var", { 7 } },       { "apple", { 3, 6, 8 } }, { "armani", { 3 } },
        { "google", { 3, 6, 10 } },     { "gucci", { 3 } },       { "nerf", { 3, 6, 10 } },
        { "roblox", { 3, 6, 10, 11 } }, { "some_var", {} },       { "y", { 1 } }
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        STATEMENT_NUMBER_LIST expected = p.second;
        STATEMENT_NUMBER_LIST actual = pkb.getStatementsThatUse(input);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getStatementsThatUseSomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_LIST expected = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    STATEMENT_NUMBER_LIST actual = pkb.getStatementsThatUseSomeVariable();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getProceduresThatUse") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_LIST> testCases = {
        { "another_var", { "caller" } },
        { "apple", { "callee", "caller" } },
        { "armani", { "caller" } },
        { "google", { "callee", "caller" } },
        { "gucci", { "caller" } },
        { "nerf", { "callee", "caller" } },
        { "roblox", { "callee", "caller" } },
        { "some_var", {} },
        { "y", { "caller" } }
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        PROCEDURE_NAME_LIST expected = p.second;
        PROCEDURE_NAME_LIST actual = pkb.getProceduresThatUse(input);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getProceduresThatUseSomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    PROCEDURE_NAME_LIST expected = { "callee", "caller" };
    PROCEDURE_NAME_LIST actual = pkb.getProceduresThatUseSomeVariable();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesUsedIn Statement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_LIST> testCases = {
        { 1, { "y" } },
        { 2, {} },
        { 3, { "apple", "armani", "google", "gucci", "microsoft", "nerf", "roblox" } },
        { 4, {} },
        { 5, {} },
        { 6, { "apple", "google", "microsoft", "nerf", "roblox" } },
        { 7, { "another_var" } },
        { 8, { "apple" } },
        { 9, { "microsoft" } },
        { 10, { "google", "nerf", "roblox" } },
        { 11, { "roblox" } }
    };
    for (auto& p : testCases) {
        auto input = p.first;
        auto expected = p.second;
        auto actual = pkb.getVariablesUsedIn(input);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesUsedBySomeProcedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "another_var", "apple", "armani", "google", "gucci",
                                    "microsoft",   "nerf",  "roblox", "y" };
    VARIABLE_NAME_LIST actual = pkb.getVariablesUsedBySomeProcedure();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesUsedIn Procedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_LIST> testCases = {
        { "caller", { "another_var", "apple", "armani", "google", "gucci", "microsoft", "nerf", "roblox", "y" } },
        { "callee", { "apple", "google", "microsoft", "nerf", "roblox" } }
    };
    for (auto& p : testCases) {
        auto input = p.first;
        auto expected = p.second;
        auto actual = pkb.getVariablesUsedIn(input);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesUsedBySomeStatement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "another_var", "apple", "armani", "google", "gucci",
                                    "microsoft",   "nerf",  "roblox", "y" };
    VARIABLE_NAME_LIST actual = pkb.getVariablesUsedBySomeStatement();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAllAssignmentStatementsThatMatch") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> actual1 = pkb.getAllAssignmentStatementsThatMatch("", "23+another_var", true);
    std::vector<int> expected1 = { 6 };
    REQUIRE(actual1 == expected1);

    std::vector<int> actual2 = pkb.getAllAssignmentStatementsThatMatch("", "23+another_var", false);
    std::vector<int> expected2 = { 6 };
    REQUIRE(actual2 == expected2);

    std::vector<int> actual3 = pkb.getAllAssignmentStatementsThatMatch("", "another_var", true);
    std::vector<int> expected3 = { 6 };
    REQUIRE(actual3 == expected3);

    std::vector<int> actual4 = pkb.getAllAssignmentStatementsThatMatch("", "another_var", false);
    std::vector<int> expected4 = {};
    REQUIRE(actual4 == expected4);

    std::vector<int> actual5 = pkb.getAllAssignmentStatementsThatMatch("some_var", "another_var", false);
    std::vector<int> expected5 = {};
    REQUIRE(actual5 == expected5);

    std::vector<int> actual6 = pkb.getAllAssignmentStatementsThatMatch("some_var", "another_var", false);
    std::vector<int> expected6 = {};
    REQUIRE(actual6 == expected6);

    std::vector<int> actual7 = pkb.getAllAssignmentStatementsThatMatch("some_var", "23+another_var", false);
    std::vector<int> expected7 = { 6 };
    REQUIRE(actual7 == expected7);
}

TEST_CASE("Test getAllAssignmentStatementsThatMatch multiple assign") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "x = p + q * r;"
                                        "x = y + q * r;"
                                        "z = y + q * r;"
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> actual1 = pkb.getAllAssignmentStatementsThatMatch("", "q*r", true);
    std::sort(actual1.begin(), actual1.end());
    std::vector<int> expected1 = { 1, 2, 3 };
    REQUIRE(actual1 == expected1);

    std::vector<int> actual2 = pkb.getAllAssignmentStatementsThatMatch("x", "q*r", true);
    std::sort(actual2.begin(), actual2.end());
    std::vector<int> expected2 = { 1, 2 };
    REQUIRE(actual2 == expected2);

    std::vector<int> actual3 = pkb.getAllAssignmentStatementsThatMatch("", "y+q", true);
    std::vector<int> expected3 = {};
    REQUIRE(actual3 == expected3);

    std::vector<int> actual4 = pkb.getAllAssignmentStatementsThatMatch("x", "y+q*r", false);
    std::vector<int> expected4 = { 2 };
    REQUIRE(actual4 == expected4);

    std::vector<int> actual5 = pkb.getAllAssignmentStatementsThatMatch("z", "y+q*r", false);
    std::vector<int> expected5 = { 3 };
    REQUIRE(actual5 == expected5);

    std::vector<int> actual6 = pkb.getAllAssignmentStatementsThatMatch("z", "y+q*r", true);
    std::vector<int> expected6 = { 3 };
    REQUIRE(actual6 == expected6);

    std::vector<int> actual7 = pkb.getAllAssignmentStatementsThatMatch("z", "y", true);
    std::vector<int> expected7 = { 3 };
    REQUIRE(actual7 == expected7);

    std::vector<int> actual8 = pkb.getAllAssignmentStatementsThatMatch("", "y", true);
    std::sort(actual8.begin(), actual8.end());
    std::vector<int> expected8 = { 2, 3 };
    REQUIRE(actual8 == expected8);

    std::vector<int> actual9 = pkb.getAllAssignmentStatementsThatMatch("x", "q*r", false);
    std::vector<int> expected9 = {};
    REQUIRE(actual9 == expected9);

    std::vector<int> actual10 =
    pkb.getAllAssignmentStatementsThatMatch("x", "impossible_string!@#$%^&*(){}|\"[0987654321", false);
    std::vector<int> expected10 = {};
    REQUIRE(actual10 == expected10);

    std::vector<int> actual11 = pkb.getAllAssignmentStatementsThatMatch("x", "1+1", false);
    std::vector<int> expected11 = {};
    REQUIRE(actual11 == expected11);

    // TODO(remo5000): https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/192
    //
    // Test modify-like pattern: getAllAssignmentStatementsThatMatch("x", "", true)
    //
    //    std::vector<int> actual10 = pkb.getAllAssignmentStatementsThatMatch("z", "", true);
    //    std::vector<int> expected10 = { 3 };
    //    REQUIRE(actual10 == expected10);
    //
    //    std::vector<int> actual11 = pkb.getAllAssignmentStatementsThatMatch("x", "", true);
    //    std::sort(actual11.begin(), actual11.end());
    //    std::vector<int> expected11 = { 1, 2 };
    //    REQUIRE(actual11 == expected11);
}
} // namespace testpkb
} // namespace backend
