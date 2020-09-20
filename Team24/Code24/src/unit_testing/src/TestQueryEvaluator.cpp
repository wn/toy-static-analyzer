#include "QEHelper.h"
#include "QueryEvaluator.h"
#include "TestQEHelper.h"
#include "catch.hpp"

namespace qpbackend {
namespace qetest {

TEST_CASE("Test argument type check provided by QEHelper") {
    // test isWildcard : check if the string is a placeholde
    REQUIRE(queryevaluator::isWildCard("_"));
    REQUIRE_FALSE(queryevaluator::isWildCard("\"_\""));

    // test isPosInt : check if the string is a positive integer
    REQUIRE(queryevaluator::isPosInt("2"));
    REQUIRE(queryevaluator::isPosInt("235629472991"));
    REQUIRE_FALSE(queryevaluator::isPosInt("name123"));
    REQUIRE_FALSE(queryevaluator::isPosInt("123name"));
    REQUIRE_FALSE(queryevaluator::isPosInt("01"));
    REQUIRE_FALSE(queryevaluator::isPosInt("-1"));

    // test isName : check if the string is a valid entity name
    REQUIRE(queryevaluator::isName("\"v\""));
    REQUIRE(queryevaluator::isName("\"V2V34EF\""));
    REQUIRE_FALSE(queryevaluator::isName("v"));
    REQUIRE_FALSE(queryevaluator::isName("\"123name\""));
    REQUIRE_FALSE(queryevaluator::isName("\"123name\""));
    REQUIRE_FALSE(queryevaluator::isName("\"proc_name\""));
}

TEST_CASE("Test with first sample source code") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // select without clauses
    Query query;
    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = {};
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14");

    query.declarationMap = { { "v", VARIABLE } };
    query.synonymsToReturn = { "v" };
    query.suchThatClauses = {};
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "count, cenX, cenY, x, y, flag, normSq");

    query.declarationMap = { { "p", PROCEDURE } };
    query.synonymsToReturn = { "p" };
    query.suchThatClauses = {};
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "computeCentroid");

    // select with Follows
    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = { { FOLLOWS, "6", "s" } };
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "7");

    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = { { FOLLOWS, "s", "4" } };
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "3");

    // select with FollowsT
    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = { { FOLLOWST, "14", "s" } };
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "");

    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = { { FOLLOWST, "s", "1" } };
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "");

    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = { { FOLLOWST, "s", "_" } };
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "1, 2, 3, 4, 5, 6, 7, 8, 10, 12");

    query.declarationMap = { { "s", STMT } };
    query.synonymsToReturn = { "s" };
    query.suchThatClauses = { { FOLLOWST, "_", "s" } };
    query.patternClauses = {};

    REQUIRE(qe.evaluateQuery(query) == "2, 3, 4, 5, 7, 8, 9, 10, 13, 14");
}

} // namespace qetest
} // namespace qpbackend
