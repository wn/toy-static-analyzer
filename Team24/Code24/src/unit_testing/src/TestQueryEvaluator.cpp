#include "QEHelper.h"
#include "QueryEvaluator.h"
#include "TestQEHelper.h"
#include "catch.hpp"

namespace qpbackend {
namespace qetest {

TEST_CASE("Test wildcard check in QEHelper") {
    REQUIRE(queryevaluator::isWildCard("_"));
    REQUIRE_FALSE(queryevaluator::isWildCard("\"_\""));
}

TEST_CASE("Test positive integer check in QEHelper") {
    REQUIRE(queryevaluator::isPosInt("2"));
    REQUIRE(queryevaluator::isPosInt("235629472991"));
    REQUIRE_FALSE(queryevaluator::isPosInt("name123"));
    REQUIRE_FALSE(queryevaluator::isPosInt("123name"));
    REQUIRE_FALSE(queryevaluator::isPosInt("01"));
    REQUIRE_FALSE(queryevaluator::isPosInt("-1"));
}

TEST_CASE("Test name check in QEHelper") {
    REQUIRE(queryevaluator::isName("\"v\""));
    REQUIRE(queryevaluator::isName("\"V2V34EF\""));
    REQUIRE_FALSE(queryevaluator::isName("v"));
    REQUIRE_FALSE(queryevaluator::isName("\"123name\""));
    REQUIRE_FALSE(queryevaluator::isName("\"123name\""));
    REQUIRE_FALSE(queryevaluator::isName("\"proc_name\""));
}

TEST_CASE("Test without clauses") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryStmt = { { { "s", STMT } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryStmt),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));

    // select variables
    Query queryVar = { { { "v", VARIABLE } }, { "v" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryVar),
                                       "count, cenX, cenY, x, y, flag, normSq"));

    // select procedures
    Query queryProc = { { { "p", PROCEDURE } }, { "p" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryProc), "computeCentroid"));
}

TEST_CASE("Test evaluation of Follows between synonyms") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { FOLLOWS, "s1", "s2" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), "1, 2, 3, 4, 5, 6, 7, 8, 10, 12"));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } }, { "s2" }, { { FOLLOWS, "s1", "s2" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), "2, 3, 4, 5, 7, 8, 9, 10, 13, 14"));
}

TEST_CASE("Test evaluation of Follows between entity and synonym") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "6", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), "7"));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "s", "4" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), "3"));
}

TEST_CASE("Test evaluation of Follows between entities") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "7", "8" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "7", "9" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows between synonym and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "s", "_" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), "1, 2, 3, 4, 5, 6, 7, 8, 10, 12"));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "_", "s" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), "2, 3, 4, 5, 7, 8, 9, 10, 13, 14"));
}

TEST_CASE("Test evaluation of Follows between entity and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "7", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "_", "6" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows(_, _)") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "_", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));
}

TEST_CASE("Test evaluation of Follows with invalid arguments") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "name1", "name2" } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Follows* between synonyms") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { FOLLOWST, "s1", "s2" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), "1, 2, 3, 4, 5, 6, 7, 8, 10, 12"));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } }, { "s2" }, { { FOLLOWST, "s1", "s2" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), "2, 3, 4, 5, 7, 8, 9, 10, 13, 14"));
}

TEST_CASE("Test evaluation of Follows* between entity and synonym") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "6", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), "7, 8, 9"));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "s", "9" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), "6, 7, 8"));
}

TEST_CASE("Test evaluation of Follows* between entities") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "7", "9" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "1", "9" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows* between synonym and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "s", "_" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), "1, 2, 3, 4, 5, 6, 7, 8, 10, 12"));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "_", "s" } }, {} };
    REQUIRE(
    checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), "2, 3, 4, 5, 7, 8, 9, 10, 13, 14"));
}

TEST_CASE("Test evaluation of Follows* between entity and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "7", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "_", "6" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows*(_, _)") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "_", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query),
                                       "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14"));
}

TEST_CASE("Test evaluation of Follows* with invalid arguments") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "name1", "name2" } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

} // namespace qetest
} // namespace qpbackend
