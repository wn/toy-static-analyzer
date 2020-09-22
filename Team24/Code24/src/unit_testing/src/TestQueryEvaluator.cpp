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
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryStmt), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    // select variables
    Query queryVar = { { { "v", VARIABLE } }, { "v" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryVar),
                                       { "count", "cenX", "cenY", "x", "y", "flag", "normSq" }));

    // select procedures
    Query queryProc = { { { "p", PROCEDURE } }, { "p" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryProc), { "computeCentroid" }));
}

TEST_CASE("TEST select entities by type") {
    PKBMock pkb(3);
    queryevaluator::QueryEvaluator qe(&pkb);

    // test select read
    Query queryRead = { { { "s", READ } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryRead), { "4", "5" }));

    // test select print
    Query queryPrint = { { { "s", PRINT } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPrint), { "6", "7", "8", "9" }));

    // test select call
    Query queryCall = { { { "s", CALL } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryCall), { "2", "3", "13", "18" }));

    // test select while
    Query queryWhile = { { { "s", WHILE } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryWhile), { "14" }));

    // test select ifelse
    Query queryIf = { { { "s", IF } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryIf), { "19" }));

    // test select isAssign
    Query queryAssign = { { { "s", ASSIGN } }, { "s" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryAssign),
                                       { "1", "10", "11", "12", "15", "16", "17", "20", "21", "22", "23" }));
}

TEST_CASE("Test evaluation of Follows between synonyms") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { FOLLOWS, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } }, { "s2" }, { { FOLLOWS, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows between entity and synonym") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "6", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "7" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "s", "4" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "3" }));
}

TEST_CASE("Test evaluation of Follows between entities") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "7", "8" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "7", "9" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows between synonym and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "s", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "_", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows between entity and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "7", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "_", "6" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows(_, _)") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "_", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                  "9", "10", "11", "12", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows with invalid arguments") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } }, {}, { { FOLLOWS, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } }, { "s1" }, { { FOLLOWS, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = { { { "s", CONSTANT } }, { "s" }, { { FOLLOWS, "1", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = { { { "s", VARIABLE } }, { "s" }, { { FOLLOWS, "s", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    // invalid entity
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "name1", "name2" } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Follows* between synonyms") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { FOLLOWST, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } }, { "s2" }, { { FOLLOWST, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows* between entity and synonym") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "6", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "7", "8", "9" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "s", "9" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "6", "7", "8" }));
}

TEST_CASE("Test evaluation of Follows* between entities") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "7", "9" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "1", "9" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows* between synonym and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "s", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "_", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows* between entity and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "7", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "_", "6" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows*(_, _)") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "_", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                  "9", "10", "11", "12", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows* with invalid arguments") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } }, {}, { { FOLLOWST, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } }, { "s1" }, { { FOLLOWST, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = { { { "s", CONSTANT } }, { "s" }, { { FOLLOWST, "1", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = { { { "s", VARIABLE } }, { "s" }, { { FOLLOWST, "s", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "name1", "name2" } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Parent between synonyms") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { PARENT, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } }, { "s2" }, { { PARENT, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));
}


TEST_CASE("Test evaluation of Parent between entity and synonym") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENT, "1", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "2", "4" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENT, "s", "5" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "4" }));
}


TEST_CASE("Test evaluation of Parent between entities") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENT, "2", "3" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENT, "1", "5" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent between synonym and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENT, "s", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENT, "_", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent between entity and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENT, "2", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENT, "_", "1" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent(_, _)") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { PARENT, "_", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent with invalid arguments") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } }, {}, { { PARENT, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } }, { "s1" }, { { PARENT, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = { { { "s", CONSTANT } }, { "s" }, { { PARENT, "1", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = { { { "s", VARIABLE } }, { "s" }, { { PARENT, "s", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    Query query = { { { "s", STMT } }, { "s" }, { { PARENT, "name1", "name2" } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Parent* between synonyms") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { PARENTT, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } }, { "s2" }, { { PARENTT, "s1", "s2" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));
}


TEST_CASE("Test evaluation of Parent* between entity and synonym") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENTT, "1", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "2", "3", "4", "5", "6" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENTT, "s", "5" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "1", "4" }));
}


TEST_CASE("Test evaluation of Parent* between entities") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENTT, "1", "5" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENTT, "5", "1" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent* between synonym and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENTT, "s", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENTT, "_", "s" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent* between entity and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENTT, "2", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENTT, "_", "1" } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent*(_, _)") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { PARENTT, "_", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent* with invalid arguments") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } }, {}, { { PARENTT, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } }, { "s1" }, { { PARENTT, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = { { { "s", CONSTANT } }, { "s" }, { { PARENTT, "1", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = { { { "s", VARIABLE } }, { "s" }, { { PARENTT, "s", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    Query query = { { { "s", STMT } }, { "s" }, { { PARENTT, "name1", "name2" } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

} // namespace qetest
} // namespace qpbackend
