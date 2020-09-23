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

    // select constants
    Query queryConst = { { { "c", CONSTANT } }, { "c" }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryConst), { "0", "1" }));
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

    Query querySelf = { { { "s", STMT } }, { "s" }, { { FOLLOWS, "s", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
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

    Query querySelf = { { { "s", STMT } }, { "s" }, { { FOLLOWST, "s", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
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

    Query querySelf = { { { "s", STMT } }, { "s" }, { { PARENT, "s", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
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

    Query querySelf = { { { "s", STMT } }, { "s" }, { { PARENTT, "s", "s" } }, {} };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
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

TEST_CASE("Test evaluation of Uses between synonyms") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesStmt = { { { "s", STMT }, { "v", VARIABLE } }, { "s" }, { { USES, "s", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesStmt), { "1", "2", "4", "5", "6" }));
    Query queryUsesVar = { { { "s", STMT }, { "v", VARIABLE } }, { "v" }, { { USES, "s", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesVar), { "x", "z", "m", "n", "count" }));

    Query queryUsepProc = { { { "p", PROCEDURE }, { "v", VARIABLE } }, { "p" }, { { USES, "p", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepProc), { "foo", "bar" }));
    Query queryUsepVar = { { { "p", PROCEDURE }, { "v", VARIABLE } }, { "v" }, { { USES, "p", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepVar), { "x", "z", "m", "n", "count" }));
}

TEST_CASE("Test evaluation of Uses between entity and synonym") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesStmt = { { { "s", STMT } }, { "s" }, { { USES, "s", "\"n\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesStmt), { "2", "4", "5" }));
    Query queryUsesVar = { { { "v", VARIABLE } }, { "v" }, { { USES, "2", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesVar), { "z", "m", "n", "count" }));

    Query queryUsepProc = { { { "p", PROCEDURE } }, { "p" }, { { USES, "p", "\"x\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepProc), { "foo" }));
    Query queryUsepVar = { { { "v", VARIABLE } }, { "v" }, { { USES, "\"bar\"", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepVar), { "z", "m", "n", "count" }));
}

TEST_CASE("Test evaluation of Uses between entities") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesTrue = { { { "s", STMT } }, { "s" }, { { USES, "5", "\"n\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsesFalse = { { { "s", STMT } }, { "s" }, { { USES, "5", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryUsesFalse).empty());

    Query queryUsepTrue = { { { "s", STMT } }, { "s" }, { { USES, "\"foo\"", "\"x\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsepFalse = { { { "s", STMT } }, { "s" }, { { USES, "\"bar\"", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryUsepFalse).empty());
}

TEST_CASE("Test evaluation of Uses between synonym and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesS = { { { "s", STMT } }, { "s" }, { { USES, "s", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesS), { "1", "2", "4", "5", "6" }));

    Query queryUseP = { { { "p", PROCEDURE } }, { "p" }, { { USES, "p", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUseP), { "foo", "bar" }));
}

TEST_CASE("Test evaluation of Uses between entity and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesTrue = { { { "s", STMT } }, { "s" }, { { USES, "5", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsesFalse = { { { "s", STMT } }, { "s" }, { { USES, "3", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(queryUsesFalse).empty());

    Query queryUsepTrue = { { { "s", STMT } }, { "s" }, { { USES, "\"foo\"", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsepFalse = { { { "s", STMT } }, { "s" }, { { USES, "\"main\"", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(queryUsepFalse).empty());
}

TEST_CASE("Test evaluation of Uses with invalid arguments") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query queryEmpty = { { { "s", STMT }, { "V", VARIABLE } }, {}, { { USES, "s", "V" } }, {} };
    REQUIRE(qe.evaluateQuery(queryEmpty).empty());

    // invalid synonym
    Query queryProc = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { USES, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(queryProc).empty());

    Query queryConst = { { { "c", CONSTANT } }, { "c" }, { { USES, "c", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryConst).empty());

    // invalid entity
    Query queryE1 = { { { "s", STMT } }, { "s" }, { { USES, "\"1\"", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryE1).empty());
    Query queryE2 = { { { "s", STMT } }, { "s" }, { { USES, "1", "x" } }, {} };
    REQUIRE(qe.evaluateQuery(queryE2).empty());

    // invalid wildcard
    Query queryE3 = { { { "s", STMT } }, { "s" }, { { USES, "_", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryE3).empty());
}

TEST_CASE("Test evaluation of Modifies between synonyms") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsStmt = { { { "s", STMT }, { "v", VARIABLE } }, { "s" }, { { MODIFIES, "s", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsStmt),
                                       { "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsesVar = { { { "s", STMT }, { "v", VARIABLE } }, { "v" }, { { MODIFIES, "s", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesVar), { "a", "n", "random", "y" }));

    Query queryModpProc = { { { "p", PROCEDURE }, { "v", VARIABLE } }, { "p" }, { { MODIFIES, "p", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpProc), { "foo", "bar" }));

    Query queryModIf = { { { "ifs", IF }, { "v", VARIABLE } }, { "ifs" }, { { MODIFIES, "ifs", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModIf), { "4" }));
    Query queryModWhile = { { { "w", WHILE }, { "v", VARIABLE } }, { "w" }, { { MODIFIES, "w", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModWhile), { "7", "8" }));

    Query queryModpVar = { { { "p", PROCEDURE }, { "v", VARIABLE } }, { "v" }, { { MODIFIES, "p", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpVar), { "a", "n", "random", "y" }));
}

TEST_CASE("Test evaluation of Modifies between entity and synonym") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsStmt = { { { "s", STMT } }, { "s" }, { { MODIFIES, "s", "\"n\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsStmt), { "2", "4", "5" }));
    Query queryModsVar = { { { "v", VARIABLE } }, { "v" }, { { MODIFIES, "2", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsVar), { "n", "random", "y" }));

    Query queryModpProc = { { { "p", PROCEDURE } }, { "p" }, { { MODIFIES, "p", "\"y\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpProc), { "foo", "bar" }));
    Query queryModpVar = { { { "v", VARIABLE } }, { "v" }, { { MODIFIES, "\"bar\"", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpVar), { "a", "n", "random", "y" }));
}

TEST_CASE("Test evaluation of Modifies between entities") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsTrue = { { { "s", STMT } }, { "s" }, { { MODIFIES, "5", "\"n\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModsFalse = { { { "s", STMT } }, { "s" }, { { MODIFIES, "5", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryModsFalse).empty());

    Query queryModpTrue = { { { "s", STMT } }, { "s" }, { { MODIFIES, "\"foo\"", "\"y\"" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModpFalse = { { { "s", STMT } }, { "s" }, { { MODIFIES, "\"foo\"", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryModpFalse).empty());
}

TEST_CASE("Test evaluation of Modifies between synonym and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesS = { { { "s", STMT } }, { "s" }, { { MODIFIES, "s", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesS), { "2", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryUseP = { { { "p", PROCEDURE } }, { "p" }, { { MODIFIES, "p", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUseP), { "foo", "bar" }));
}

TEST_CASE("Test evaluation of Modifies between entity and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsTrue = { { { "s", STMT } }, { "s" }, { { MODIFIES, "5", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModsFalse = { { { "s", STMT } }, { "s" }, { { MODIFIES, "1", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(queryModsFalse).empty());

    Query queryModpTrue = { { { "s", STMT } }, { "s" }, { { MODIFIES, "\"foo\"", "_" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModpFalse = { { { "s", STMT } }, { "s" }, { { MODIFIES, "\"main\"", "_" } }, {} };
    REQUIRE(qe.evaluateQuery(queryModpFalse).empty());
}

TEST_CASE("Test evaluation of Modifies with invalid arguments") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query queryEmpty = { { { "s", STMT }, { "V", VARIABLE } }, {}, { { MODIFIES, "s", "V" } }, {} };
    REQUIRE(qe.evaluateQuery(queryEmpty).empty());

    // invalid synonym
    Query queryProc = { { { "s1", STMT }, { "s2", STMT } }, { "s1" }, { { MODIFIES, "s1", "s2" } }, {} };
    REQUIRE(qe.evaluateQuery(queryProc).empty());

    Query queryConst = { { { "c", CONSTANT } }, { "c" }, { { MODIFIES, "c", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryConst).empty());

    // invalid entity
    Query queryE1 = { { { "s", STMT } }, { "s" }, { { MODIFIES, "\"1\"", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryE1).empty());
    Query queryE2 = { { { "s", STMT } }, { "s" }, { { MODIFIES, "1", "x" } }, {} };
    REQUIRE(qe.evaluateQuery(queryE2).empty());

    // invalid wildcard
    Query queryE3 = { { { "s", STMT } }, { "s" }, { { MODIFIES, "_", "\"x\"" } }, {} };
    REQUIRE(qe.evaluateQuery(queryE3).empty());
}

TEST_CASE("Test combined Uses and Modifies") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUse = { { { "s", STMT }, { "v", VARIABLE } }, { "s" }, { { USES, "s", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUse), { "1", "2", "4", "5", "6" }));

    Query queryMod = { { { "s", STMT }, { "v", VARIABLE } }, { "s" }, { { MODIFIES, "s", "v" } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryMod), { "2", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryDouble = {
        { { "s", STMT }, { "v", VARIABLE } }, { "s" }, { { MODIFIES, "s", "v" }, { USES, "s", "v" } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryDouble), { "2", "4", "5" }));
}

TEST_CASE("Test pattern check") {
    bool isPattern, isSubExpr;
    std::string patternStr;

    // pattern string without double quote on both sides
    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("x+y");
    REQUIRE_FALSE(isPattern);
    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("\"x+y");
    REQUIRE_FALSE(isPattern);

    // single underscore on one side
    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("\"x+y\"_");
    REQUIRE_FALSE(isPattern);

    // no double quote between underscores
    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("_x+y_");
    REQUIRE_FALSE(isPattern);
    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("_x+y\"_");
    REQUIRE_FALSE(isPattern);

    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("\"x+y\"");
    REQUIRE(isPattern);
    REQUIRE(patternStr == "x+y");
    REQUIRE_FALSE(isSubExpr);

    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("_\"x+y\"_");
    REQUIRE(isPattern);
    REQUIRE(patternStr == "x+y");
    REQUIRE(isSubExpr);

    std::tie(isPattern, patternStr, isSubExpr) = queryevaluator::extractPatternExpr("_");
    REQUIRE(isPattern);
    REQUIRE(patternStr.empty());
    REQUIRE(isSubExpr);
}

TEST_CASE("Test pattern of exact match") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } }, { "a" }, {}, { { "a", "_", "\"0\"" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "1", "2", "3" }));

    Query query2 = { { { "a", ASSIGN }, { "cl", CALL } }, { "cl" }, {}, { { "a", "\"cenX\"", "\"cenX+x\"" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "4", "9" }));

    Query query3 = { { { "a", ASSIGN }, { "v", VARIABLE } }, { "v" }, {}, { { "a", "v", "\"cenX+x\"" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX" }));
}

TEST_CASE("Test pattern of subexpression") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } }, { "a" }, {}, { { "a", "_", "_\"count\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "6", "12", "13" }));

    Query query2 = { { { "a", ASSIGN }, { "cl", CALL } }, { "a" }, {}, { { "a", "\"count\"", "_\"0\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "1" }));

    Query query3 = { { { "a", ASSIGN }, { "v", VARIABLE } }, { "v" }, {}, { { "a", "v", "_\"0\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX", "cenY", "count" }));
}

TEST_CASE("Test pattern of wildcard pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } }, { "a" }, {}, { { "a", "_", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1),
                                       { "1", "2", "3", "6", "7", "8", "11", "12", "13", "14" }));

    Query query2 = { { { "a", ASSIGN } }, { "a" }, {}, { { "a", "\"cenX\"", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "2", "7", "12" }));

    Query query3 = { { { "a", ASSIGN }, { "v", VARIABLE } }, { "v" }, {}, { { "a", "v", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX", "cenY", "count", "normSq", "flag" }));
}

TEST_CASE("Test invalid pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // the statment should be an assignment
    Query query1 = { { { "a", STMT } }, { "a" }, {}, { { "a", "_", "_" } } };
    REQUIRE(qe.evaluateQuery(query1).empty());

    // assignment should be a synonym
    Query query2 = { { { "v", VARIABLE } }, { "v" }, {}, { { "1", "v", "_" } } };
    REQUIRE(qe.evaluateQuery(query2).empty());

    // invalid pattern
    Query query3 = { { { "a", ASSIGN } }, { "a" }, {}, { { "a", "_", "0" } } };
    REQUIRE(qe.evaluateQuery(query3).empty());
}

TEST_CASE("Test Follows/Follows* and Pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = {
        { { "s", STMT }, { "a", ASSIGN } }, { "s" }, { { FOLLOWST, "s", "a" } }, { { "a", "\"cenY\"", "_" } }
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "1", "2", "6", "7", "12" }));

    Query query2 = { { { "a", ASSIGN } }, { "a" }, { { FOLLOWST, "_", "a" } }, { { "a", "\"count\"", "_" } } };
    REQUIRE(qe.evaluateQuery(query2).empty());

    Query query3 = { { { "v", VARIABLE }, { "a", ASSIGN } }, { "v" }, { { FOLLOWS, "6", "a" } }, { { "a", "v", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX" }));
}

TEST_CASE("Test Parent/Parent* and Pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "s", STMT }, { "a", ASSIGN } }, { "a" }, { { PARENTT, "s", "a" } }, { { "a", "_", "_\"cenX\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "7", "12" }));

    Query query2 = { { { "a", ASSIGN }, { "w", WHILE } }, { "a" }, { { PARENT, "w", "a" } }, { { "a", "\"count\"", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "6" }));

    Query query3 = { { { "ifs", IF }, { "v", VARIABLE }, { "a", ASSIGN } },
                     { "v" },
                     { { PARENT, "ifs", "a" } },
                     { { "a", "v", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "flag", "cenX", "cenY" }));
}

TEST_CASE("Test Uses and Pattern") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN }, { "v", VARIABLE } }, { "a" }, { { USES, "a", "v" } }, { { "a", "v", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "5" }));

    Query query2 = { { { "a", ASSIGN }, { "ifs", IF }, { "v", VARIABLE } },
                     { "a" },
                     { { USES, "ifs", "v" } },
                     { { "a", "v", "_\"1\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "5" }));

    Query query3 = { { { "a", ASSIGN } }, { "a" }, { { USES, "a", "\"m\"" } }, { { "a", "_", "_\"z\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "6" }));
}

TEST_CASE("Test Modifies and Pattern") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } }, { "a" }, { { MODIFIES, "a", "_" } }, { { "a", "\"y\"", "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "6" }));

    Query query2 = { { { "a", ASSIGN }, { "ifs", IF }, { "v", VARIABLE } },
                     { "a" },
                     { { MODIFIES, "ifs", "v" } },
                     { { "a", "v", "_\"1\"_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "5" }));
}

} // namespace qetest
} // namespace qpbackend
