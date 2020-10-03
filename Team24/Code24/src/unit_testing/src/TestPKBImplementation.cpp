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
template <typename T> std::vector<T> GetSortedVector(std::vector<T> v) {
    sort(v.begin(), v.end());
    return v;
}

TEST_CASE("Test getDirectFollow") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = { 3 };
    REQUIRE(pkb.getDirectFollow(1) == expected);

    std::vector<int> expected2 = {};
    REQUIRE(pkb.getDirectFollow(2) == expected2);

    std::vector<int> expected3 = { 6 };
    REQUIRE(pkb.getDirectFollow(3) == expected3);

    std::vector<int> expected4 = {};
    REQUIRE(pkb.getDirectFollow(4) == expected4);

    std::vector<int> expected5 = {};
    REQUIRE(pkb.getDirectFollow(5) == expected5);

    std::vector<int> expected6 = {};
    REQUIRE(pkb.getDirectFollow(6) == expected6);
}

TEST_CASE("Test getDirectFollowedBy") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = {};
    REQUIRE(pkb.getDirectFollowedBy(1) == expected);

    std::vector<int> expected2 = {};
    REQUIRE(pkb.getDirectFollowedBy(2) == expected2);

    std::vector<int> expected3 = { 1 };
    REQUIRE(pkb.getDirectFollowedBy(3) == expected3);

    std::vector<int> expected4 = {};
    REQUIRE(pkb.getDirectFollowedBy(4) == expected4);

    std::vector<int> expected5 = {};
    REQUIRE(pkb.getDirectFollowedBy(5) == expected5);

    std::vector<int> expected6 = { 3 };
    REQUIRE(pkb.getDirectFollowedBy(6) == expected6);
}

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
    REQUIRE(pkb.getStatementsThatFollows(1) == expected);

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

TEST_CASE("Test getParent") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "while (y == 3) {"
                                        "a=3;"
                                        "while (x == 3) {"
                                        "x = 1;"
                                        "}"
                                        "b=4;"
                                        "}"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = {};
    REQUIRE(pkb.getParent(1) == expected);

    std::vector<int> expected2 = { 1 };
    REQUIRE(pkb.getParent(2) == expected2);

    std::vector<int> expected3 = { 1 };
    REQUIRE(pkb.getParent(3) == expected3);

    std::vector<int> expected4 = { 3 };
    REQUIRE(pkb.getParent(4) == expected4);

    std::vector<int> expected5 = { 1 };
    REQUIRE(pkb.getParent(5) == expected5);
}

TEST_CASE("Test getChildren") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "while (y == 3) {"
                                        "a=3;"
                                        "while (x == 3) {"
                                        "x = 1;"
                                        "}"
                                        "b=4;"
                                        "}"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> expected = { 2, 3, 5 };
    REQUIRE(pkb.getChildren(1) == expected);

    std::vector<int> expected2 = {};
    REQUIRE(pkb.getChildren(2) == expected2);

    std::vector<int> expected3 = { 4 };
    REQUIRE(pkb.getChildren(3) == expected3);

    std::vector<int> expected4 = {};
    REQUIRE(pkb.getChildren(4) == expected4);

    std::vector<int> expected5 = {};
    REQUIRE(pkb.getChildren(5) == expected5);
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

// We use cars and modify jewellery
const char USES_AND_MODIFIES_PROGRAM[] = "procedure main {"
                                         "while (2 == nike) {" // 1
                                         "gucci = adidas;" // 2
                                         "}"
                                         "call callee1;" // 3
                                         "call callee2;" // 4
                                         "}\n"
                                         "procedure callee1 {"
                                         "if (3 == 3) then {" // 5
                                         "pasha = yeezy;" //  6
                                         "} else {"
                                         "print newBalance;" // 7
                                         "}"
                                         "}"
                                         "procedure callee2 {"
                                         "read armani;" // 8
                                         "}";

TEST_CASE("Test getStatementsThatUse") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_LIST> testCases = {
        { "nike", { 1 } }, { "adidas", { 1, 2 } }, { "yeezy", { 3, 5, 6 } }, { "newBalance", { 3, 5, 7 } }
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        STATEMENT_NUMBER_LIST expected = p.second;
        STATEMENT_NUMBER_LIST actual = GetSortedVector(pkb.getStatementsThatUse(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getStatementsThatUseSomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_LIST expected = { 1, 2, 3, 5, 6, 7 };
    STATEMENT_NUMBER_LIST actual = GetSortedVector(pkb.getStatementsThatUseSomeVariable());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getProceduresThatUse") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_LIST> testCases = {
        { "nike", { "main" } },
        { "adidas", { "main" } },
        { "yeezy", { "callee1", "main" } },
        { "newBalance", { "callee1", "main" } },
        { "gucci", {} },
        { "pasha", {} },
        { "armani", {} },
        { "nonexistentvariable", {} },
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        PROCEDURE_NAME_LIST expected = p.second;
        PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatUse(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getProceduresThatUseSomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    PROCEDURE_NAME_LIST expected = { "callee1", "main" };
    PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatUseSomeVariable());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesUsedIn Statement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_LIST> testCases = {
        { 1, { "adidas", "nike" } },      { 2, { "adidas" } },
        { 3, { "newBalance", "yeezy" } }, { 4, {} },
        { 5, { "newBalance", "yeezy" } }, { 6, { "yeezy" } },
        { 7, { "newBalance" } },          { 8, {} }
    };
    for (auto& p : testCases) {
        STATEMENT_NUMBER input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedIn(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesUsedBySomeProcedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "adidas", "newBalance", "nike", "yeezy" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedBySomeProcedure());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesUsedIn Procedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_LIST> testCases = {
        { "main", { "adidas", "newBalance", "nike", "yeezy" } },
        { "callee1", { "newBalance", "yeezy" } },
        { "callee2", {} }
    };
    for (auto& p : testCases) {
        PROCEDURE_NAME input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedIn(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesUsedBySomeStatement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "adidas", "newBalance", "nike", "yeezy" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedBySomeStatement());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getStatementsThatModify") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_LIST> testCases = {
        { "nike", {} },         { "adidas", {} },
        { "yeezy", {} },        { "newBalance", {} },
        { "gucci", { 1, 2 } },  { "pasha", { 3, 5, 6 } },
        { "armani", { 4, 8 } }, { "nonexistentvariable", {} },
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        STATEMENT_NUMBER_LIST expected = p.second;
        STATEMENT_NUMBER_LIST actual = GetSortedVector(pkb.getStatementsThatModify(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getStatementsThatModifySomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_LIST expected = { 1, 2, 3, 4, 5, 6, 8 };
    STATEMENT_NUMBER_LIST actual = GetSortedVector(pkb.getStatementsThatModifySomeVariable());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getProceduresThatModify") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_LIST> testCases = {
        { "nike", {} },
        { "adidas", {} },
        { "yeezy", {} },
        { "newBalance", {} },
        { "gucci", { "main" } },
        { "pasha", { "callee1", "main" } },
        { "armani", { "callee2", "main" } },
        { "nonexistentvariable", {} },
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        PROCEDURE_NAME_LIST expected = p.second;
        PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatModify(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getProceduresThatModifySomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    PROCEDURE_NAME_LIST expected = { "callee1", "callee2", "main" };
    PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatModifySomeVariable());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesModifiedBy Statement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_LIST> testCases = {
        { 1, { "gucci" } }, { 2, { "gucci" } }, { 3, { "pasha" } }, { 4, { "armani" } },
        { 5, { "pasha" } }, { 6, { "pasha" } }, { 7, {} },          { 8, { "armani" } },
    };
    for (auto& p : testCases) {
        STATEMENT_NUMBER input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBy(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesModifiedBySomeProcedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "armani", "gucci", "pasha" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBySomeProcedure());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesModifiedBy Procedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_LIST> testCases = {
        { "main", { "armani", "gucci", "pasha" } }, { "callee1", { "pasha" } }, { "callee2", { "armani" } }
    };
    for (auto& p : testCases) {
        PROCEDURE_NAME input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBy(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesModifiedBySomeStatement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "armani", "gucci", "pasha" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBySomeStatement());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAllAssignmentStatementsThatMatch") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> actual1 = pkb.getAllAssignmentStatementsThatMatch("_", "23+another_var", true);
    std::vector<int> expected1 = { 6 };
    REQUIRE(actual1 == expected1);

    std::vector<int> actual2 = pkb.getAllAssignmentStatementsThatMatch("_", "23+another_var", false);
    std::vector<int> expected2 = { 6 };
    REQUIRE(actual2 == expected2);

    std::vector<int> actual3 = pkb.getAllAssignmentStatementsThatMatch("_", "another_var", true);
    std::vector<int> expected3 = { 6 };
    REQUIRE(actual3 == expected3);

    std::vector<int> actual4 = pkb.getAllAssignmentStatementsThatMatch("_", "another_var", false);
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

    std::vector<int> actual8 = pkb.getAllAssignmentStatementsThatMatch("some_var", "23", true);
    std::vector<int> expected8 = { 6 };
    REQUIRE(actual8 == expected8);
}

TEST_CASE("Test getAllAssignmentStatementsThatMatch multiple assign") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "x = p + q * r;"
                                        "x = y + q * r;"
                                        "z = y + q * r;"
                                        "w = p + q + r;"
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> actual1 = GetSortedVector(pkb.getAllAssignmentStatementsThatMatch("_", "q*r", true));
    std::vector<int> expected1 = { 1, 2, 3 };
    REQUIRE(actual1 == expected1);

    std::vector<int> actual2 = GetSortedVector(pkb.getAllAssignmentStatementsThatMatch("x", "q*r", true));
    std::vector<int> expected2 = { 1, 2 };
    REQUIRE(actual2 == expected2);

    std::vector<int> actual3 = pkb.getAllAssignmentStatementsThatMatch("_", "y+q", true);
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

    std::vector<int> actual8 = GetSortedVector(pkb.getAllAssignmentStatementsThatMatch("_", "y", true));
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

    // These are implicit "Modifies" queries since patterns aren't specified.
    std::vector<int> actual12 = pkb.getAllAssignmentStatementsThatMatch("z", "", true);
    std::vector<int> expected12 = { 3 };
    REQUIRE(actual12 == expected12);

    std::vector<int> actual13 = GetSortedVector(pkb.getAllAssignmentStatementsThatMatch("x", "", true));
    std::vector<int> expected13 = { 1, 2 };
    REQUIRE(actual13 == expected13);

    // Left-assoc
    std::vector<int> actual14 = pkb.getAllAssignmentStatementsThatMatch("_", "((p+q)+r)", false);
    std::vector<int> expected14 = { 4 };
    REQUIRE(actual14 == expected14);

    std::vector<int> actual15 = pkb.getAllAssignmentStatementsThatMatch("x", "                 ", false);
    std::vector<int> expected15 = {};
    REQUIRE(actual15 == expected15);

    std::vector<int> actual16 = pkb.getAllAssignmentStatementsThatMatch("z", "y+(q*r)", true);
    std::vector<int> expected16 = { 3 };
    REQUIRE(actual16 == expected16);

    std::vector<int> actual17 = pkb.getAllAssignmentStatementsThatMatch("z", "(y+q)*r", true);
    std::vector<int> expected17 = {};
    REQUIRE(actual17 == expected17);

    std::vector<int> actual18 = GetSortedVector(pkb.getAllAssignmentStatementsThatMatch("_", "", true));
    std::vector<int> expected18 = { 1, 2, 3, 4 };
    REQUIRE(actual18 == expected18);
}

TEST_CASE("Test invalid getAllAssignmentStatementsThatMatch patterns") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "x =  q * r + 123 + 1+123;"
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<int> actual1 = pkb.getAllAssignmentStatementsThatMatch("_", "()", true);
    std::vector<int> expected1 = {};
    REQUIRE(actual1 == expected1);

    std::vector<int> actual1_f = pkb.getAllAssignmentStatementsThatMatch("_", "()", false);
    std::vector<int> expected1_f = {};
    REQUIRE(actual1_f == expected1_f);


    std::vector<int> actual2 = pkb.getAllAssignmentStatementsThatMatch("_", "()q*r()", true);
    std::vector<int> expected2 = {};
    REQUIRE(actual2 == expected2);

    std::vector<int> actual2_f = pkb.getAllAssignmentStatementsThatMatch("_", "()q*r()", false);
    std::vector<int> expected2_f = {};
    REQUIRE(actual2_f == expected2_f);

    std::vector<int> actual3 = pkb.getAllAssignmentStatementsThatMatch("_", "12 3", true);
    std::vector<int> expected3 = {};
    REQUIRE(actual3 == expected3);

    std::vector<int> actual3_f = pkb.getAllAssignmentStatementsThatMatch("_", "12 3", false);
    std::vector<int> expected3_f = {};
    REQUIRE(actual3_f == expected3_f);

    std::vector<int> actual4 = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+12\n3", true);
    std::vector<int> expected4 = {};
    REQUIRE(actual4 == expected4);

    std::vector<int> actual4_f = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+12\n3", false);
    std::vector<int> expected4_f = {};
    REQUIRE(actual4_f == expected4_f);

    std::vector<int> actual5 = pkb.getAllAssignmentStatementsThatMatch("_", "q\n*r+123", true);
    std::vector<int> expected5 = { 1 };
    REQUIRE(actual5 == expected5);

    std::vector<int> actual5_f = pkb.getAllAssignmentStatementsThatMatch("_", "q\n*r+123", false);
    std::vector<int> expected5_f = {};
    REQUIRE(actual5_f == expected5_f);

    std::vector<int> actual6 = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+\n123", true);
    std::vector<int> expected6 = { 1 };
    REQUIRE(actual6 == expected6);

    std::vector<int> actual6_f = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+\n123", false);
    std::vector<int> expected6_f = {};
    REQUIRE(actual6_f == expected6_f);

    std::vector<int> actual7 = pkb.getAllAssignmentStatementsThatMatch("_", "    ", true);
    std::vector<int> expected7 = { 1 };
    REQUIRE(actual7 == expected7);

    std::vector<int> actual7_f = pkb.getAllAssignmentStatementsThatMatch("_", "    ", false);
    std::vector<int> expected7_f = {};
    REQUIRE(actual7_f == expected7_f);

    std::vector<int> actual8 = pkb.getAllAssignmentStatementsThatMatch("_", "1+12 3", true);
    std::vector<int> expected8 = {};
    REQUIRE(actual8 == expected8);

    std::vector<int> actual8_f = pkb.getAllAssignmentStatementsThatMatch("_", "1+12 3", false);
    std::vector<int> expected8_f = {};
    REQUIRE(actual8_f == expected8_f);
}

TEST_CASE("Test isEntity") {
    const char STRUCTURED_STATEMENT[] = "procedure aoeu {"

                                        "while (y == 3) {"
                                        "gucci = 1;"
                                        "}"

                                        "if (!(armani == gucci)) then {"
                                        "read x;"
                                        "call y;"
                                        "} else {"
                                        "print z;"
                                        "}"
                                        "}"

                                        "procedure y {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    REQUIRE(pkb.isWhile(1));
    REQUIRE(pkb.isAssign(2));
    REQUIRE(pkb.isIfElse(3));
    REQUIRE(pkb.isRead(4));
    REQUIRE(pkb.isCall(5));
    REQUIRE(pkb.isPrint(6));

    REQUIRE_FALSE(pkb.isWhile(2));
    REQUIRE_FALSE(pkb.isWhile(3));
    REQUIRE_FALSE(pkb.isWhile(4));
    REQUIRE_FALSE(pkb.isWhile(5));
    REQUIRE_FALSE(pkb.isWhile(6));

    REQUIRE_FALSE(pkb.isAssign(1));
    REQUIRE_FALSE(pkb.isAssign(3));
    REQUIRE_FALSE(pkb.isAssign(4));
    REQUIRE_FALSE(pkb.isAssign(5));
    REQUIRE_FALSE(pkb.isAssign(6));

    REQUIRE_FALSE(pkb.isIfElse(1));
    REQUIRE_FALSE(pkb.isIfElse(2));
    REQUIRE_FALSE(pkb.isIfElse(4));
    REQUIRE_FALSE(pkb.isIfElse(5));
    REQUIRE_FALSE(pkb.isIfElse(6));

    REQUIRE_FALSE(pkb.isRead(1));
    REQUIRE_FALSE(pkb.isRead(2));
    REQUIRE_FALSE(pkb.isRead(3));
    REQUIRE_FALSE(pkb.isRead(5));
    REQUIRE_FALSE(pkb.isRead(6));

    REQUIRE_FALSE(pkb.isCall(1));
    REQUIRE_FALSE(pkb.isCall(2));
    REQUIRE_FALSE(pkb.isCall(3));
    REQUIRE_FALSE(pkb.isCall(4));
    REQUIRE_FALSE(pkb.isCall(6));

    REQUIRE_FALSE(pkb.isPrint(1));
    REQUIRE_FALSE(pkb.isPrint(2));
    REQUIRE_FALSE(pkb.isPrint(3));
    REQUIRE_FALSE(pkb.isPrint(4));
    REQUIRE_FALSE(pkb.isPrint(5));
}

TEST_CASE("Test getAllEntity") {
    const char STRUCTURED_STATEMENT[] = "procedure aoeu {"

                                        "while (y == 3) {"
                                        "gucci = 1;"
                                        "}"

                                        "if (!(armani == gucci)) then {"
                                        "read x;"
                                        "call y;"
                                        "} else {"
                                        "print z;"
                                        "}"
                                        "}"

                                        "procedure y {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::vector<std::string> expectedVariables = { "armani", "gucci", "x", "y", "z" };
    std::vector<std::string> actualVariables = pkb.getAllVariables();
    sort(actualVariables.begin(), actualVariables.end());
    REQUIRE(actualVariables == expectedVariables);

    std::vector<std::string> expectedProcedures = { "aoeu", "y" };
    std::vector<std::string> actualProcedures = pkb.getAllProcedures();
    sort(actualProcedures.begin(), actualProcedures.end());
    REQUIRE(actualProcedures == expectedProcedures);

    std::unordered_set<std::string> expectedConstants = { "3", "1" };
    std::unordered_set<std::string> actualConstants = pkb.getAllConstants();
    REQUIRE(actualConstants == expectedConstants);
}

TEST_CASE("Test getProcedureThatCalls") {
    const char STRUCTURED_STATEMENT[] = "procedure a {"
                                        "call b;"
                                        "call d;"
                                        "}"

                                        "procedure b {"
                                        "call c;"
                                        "call d;"
                                        "}"

                                        "procedure c {y = 1+1;}"
                                        "procedure d {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::unordered_set<std::string> actualA = pkb.getProcedureThatCalls("a", false);
    std::unordered_set<std::string> expectedA = {};
    REQUIRE(actualA == expectedA);

    std::unordered_set<std::string> actualB = pkb.getProcedureThatCalls("b", false);
    std::unordered_set<std::string> expectedB = { "a" };
    REQUIRE(actualB == expectedB);

    std::unordered_set<std::string> actualC = pkb.getProcedureThatCalls("c", false);
    std::unordered_set<std::string> expectedC = { "b" };
    REQUIRE(actualC == expectedC);

    std::unordered_set<std::string> actualD = pkb.getProcedureThatCalls("d", false);
    std::unordered_set<std::string> expectedD = { "a", "b" };
    REQUIRE(actualD == expectedD);

    std::unordered_set<std::string> actualA_transitive = pkb.getProcedureThatCalls("a", true);
    std::unordered_set<std::string> expectedA_transitive = {};
    REQUIRE(actualA_transitive == expectedA_transitive);

    std::unordered_set<std::string> actualB_transitive = pkb.getProcedureThatCalls("b", true);
    std::unordered_set<std::string> expectedB_transitive = { "a" };
    REQUIRE(actualB_transitive == expectedB_transitive);

    std::unordered_set<std::string> actualC_transitive = pkb.getProcedureThatCalls("c", true);
    std::unordered_set<std::string> expectedC_transitive = { "a", "b" };
    REQUIRE(actualC_transitive == expectedC_transitive);

    std::unordered_set<std::string> actualD_transitive = pkb.getProcedureThatCalls("d", true);
    std::unordered_set<std::string> expectedD_transitive = { "a", "b" };
    REQUIRE(actualD_transitive == expectedD_transitive);

    std::unordered_set<std::string> actualE_transitive = pkb.getProcedureThatCalls("e", true);
    std::unordered_set<std::string> expectedE_transitive = {};
    REQUIRE(actualE_transitive == expectedE_transitive);
}

TEST_CASE("Test getProceduresCalledBy") {
    const char STRUCTURED_STATEMENT[] = "procedure a {"
                                        "call b;"
                                        "call d;"
                                        "}"

                                        "procedure b {"
                                        "call c;"
                                        "call d;"
                                        "}"

                                        "procedure c {y = 1+1;}"
                                        "procedure d {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::unordered_set<std::string> actualA = pkb.getProceduresCalledBy("a", false);
    std::unordered_set<std::string> expectedA = { "b", "d" };
    REQUIRE(actualA == expectedA);

    std::unordered_set<std::string> actualB = pkb.getProceduresCalledBy("b", false);
    std::unordered_set<std::string> expectedB = { "c", "d" };
    REQUIRE(actualB == expectedB);

    std::unordered_set<std::string> actualC = pkb.getProceduresCalledBy("c", false);
    std::unordered_set<std::string> expectedC = {};
    REQUIRE(actualC == expectedC);

    std::unordered_set<std::string> actualD = pkb.getProceduresCalledBy("d", false);
    std::unordered_set<std::string> expectedD = {};
    REQUIRE(actualD == expectedD);

    std::unordered_set<std::string> actualE = pkb.getProceduresCalledBy("e", false);
    std::unordered_set<std::string> expectedE = {};
    REQUIRE(actualE == expectedE);

    std::unordered_set<std::string> actualA_transitive = pkb.getProceduresCalledBy("a", true);
    std::unordered_set<std::string> expectedA_transitive = { "b", "c", "d" };
    REQUIRE(actualA_transitive == expectedA_transitive);

    std::unordered_set<std::string> actualB_transitive = pkb.getProceduresCalledBy("b", true);
    std::unordered_set<std::string> expectedB_transitive = { "c", "d" };
    REQUIRE(actualB_transitive == expectedB_transitive);

    std::unordered_set<std::string> actualC_transitive = pkb.getProceduresCalledBy("c", true);
    std::unordered_set<std::string> expectedC_transitive = {};
    REQUIRE(actualC_transitive == expectedC_transitive);

    std::unordered_set<std::string> actualD_transitive = pkb.getProceduresCalledBy("d", true);
    std::unordered_set<std::string> expectedD_transitive = {};
    REQUIRE(actualD_transitive == expectedD_transitive);

    std::unordered_set<std::string> actualE_transitive = pkb.getProceduresCalledBy("e", true);
    std::unordered_set<std::string> expectedE_transitive = {};
    REQUIRE(actualE_transitive == expectedE_transitive);
}
} // namespace testpkb
} // namespace backend
