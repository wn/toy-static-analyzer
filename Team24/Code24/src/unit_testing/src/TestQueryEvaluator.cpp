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

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } },
                       { "s1" },
                       { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } },
                        { "s2" },
                        { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));

    Query querySelf = {
        { { "s", STMT } }, { "s" }, { { FOLLOWS, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
}

TEST_CASE("Test evaluation of Follows between entity and synonym") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "6" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "7" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { STMT_SYNONYM, "s" }, { NUM_ENTITY, "4" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "3" }));
}

TEST_CASE("Test evaluation of Follows between entities") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "7" }, { NUM_ENTITY, "8" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "7" }, { NUM_ENTITY, "9" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows between synonym and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { WILDCARD, "_" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows between entity and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "7" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { WILDCARD, "_" }, { NUM_ENTITY, "6" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows(_, _)") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { WILDCARD, "_" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                  "9", "10", "11", "12", "13", "14" }));
}


TEST_CASE("Test evaluation of Follows with invalid arguments") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } },
                          std::vector<std::string>(),
                          { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } },
                         { "s1" },
                         { { FOLLOWS, { STMT_SYNONYM, "s1" }, { PROC_SYNONYM, "s2" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = {
        { { "s", CONSTANT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "1" }, { CONST_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = {
        { { "s", VARIABLE } }, { "s" }, { { FOLLOWS, { VAR_SYNONYM, "s" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    // invalid entity
    Query query = { { { "s", STMT } },
                    { "s" },
                    { { FOLLOWS, { STMT_SYNONYM, "name1" }, { STMT_SYNONYM, "name2" } } },
                    {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Follows* between synonyms") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } },
                       { "s1" },
                       { { FOLLOWST, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } },
                        { "s2" },
                        { { FOLLOWST, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));

    Query querySelf = {
        { { "s", STMT } }, { "s" }, { { FOLLOWST, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
}

TEST_CASE("Test evaluation of Follows* between entity and synonym") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { NUM_ENTITY, "6" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "7", "8", "9" }));

    Query queryPost = {
        { { "s", STMT } }, { "s" }, { { FOLLOWST, { STMT_SYNONYM, "s" }, { NUM_ENTITY, "9" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "6", "7", "8" }));
}

TEST_CASE("Test evaluation of Follows* between entities") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { NUM_ENTITY, "7" }, { NUM_ENTITY, "9" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { NUM_ENTITY, "1" }, { NUM_ENTITY, "9" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows* between synonym and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "10", "12" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { WILDCARD, "_" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost),
                                       { "2", "3", "4", "5", "7", "8", "9", "10", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows* between entity and wildcard") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { NUM_ENTITY, "7" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                      "9", "10", "11", "12", "13", "14" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { WILDCARD, "_" }, { NUM_ENTITY, "6" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Follows*(_, _)") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { FOLLOWST, { WILDCARD, "_" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6", "7", "8",
                                                                  "9", "10", "11", "12", "13", "14" }));
}

TEST_CASE("Test evaluation of Follows* with invalid arguments") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } },
                          std::vector<std::string>(),
                          { { FOLLOWST, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } },
                         { "s1" },
                         { { FOLLOWST, { STMT_SYNONYM, "s1" }, { PROC_SYNONYM, "s2" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = {
        { { "s", CONSTANT } }, { "s" }, { { FOLLOWST, { NUM_ENTITY, "1" }, { CONST_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = {
        { { "s", VARIABLE } }, { "s" }, { { FOLLOWST, { VAR_SYNONYM, "s" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    Query query = { { { "s", STMT } },
                    { "s" },
                    { { FOLLOWST, { NAME_ENTITY, "name1" }, { NAME_ENTITY, "name2" } } },
                    {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Parent between synonyms") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } },
                       { "s1" },
                       { { PARENT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } },
                        { "s2" },
                        { { PARENT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));

    Query querySelf = {
        { { "s", STMT } }, { "s" }, { { PARENT, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
}


TEST_CASE("Test evaluation of Parent between entity and synonym") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENT, { NUM_ENTITY, "1" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "2", "4" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENT, { STMT_SYNONYM, "s" }, { NUM_ENTITY, "5" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "4" }));
}


TEST_CASE("Test evaluation of Parent between entities") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENT, { NUM_ENTITY, "2" }, { NUM_ENTITY, "3" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENT, { NUM_ENTITY, "1" }, { NUM_ENTITY, "5" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent between synonym and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENT, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENT, { WILDCARD, "_" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent between entity and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENT, { NUM_ENTITY, "2" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENT, { WILDCARD, "_" }, { NUM_ENTITY, "1" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent(_, _)") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { PARENT, { WILDCARD, "_" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent with invalid arguments") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } },
                          std::vector<std::string>(),
                          { { PARENT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } },
                         { "s1" },
                         { { PARENT, { STMT_SYNONYM, "s1" }, { PROC_SYNONYM, "s2" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = {
        { { "s", CONSTANT } }, { "s" }, { { PARENT, { NUM_ENTITY, "1" }, { CONST_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = {
        { { "s", VARIABLE } }, { "s" }, { { PARENT, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    Query query = { { { "s", STMT } },
                    { "s" },
                    { { PARENT, { STMT_SYNONYM, "name1" }, { STMT_SYNONYM, "name2" } } },
                    {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Parent* between synonyms") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } },
                       { "s1" },
                       { { PARENTT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } },
                        { "s2" },
                        { { PARENTT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));

    Query querySelf = {
        { { "s", STMT } }, { "s" }, { { PARENTT, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
}


TEST_CASE("Test evaluation of Parent* between entity and synonym") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENTT, { NUM_ENTITY, "1" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "2", "3", "4", "5", "6" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENTT, { STMT_SYNONYM, "s" }, { NUM_ENTITY, "5" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "1", "4" }));
}


TEST_CASE("Test evaluation of Parent* between entities") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENTT, { NUM_ENTITY, "1" }, { NUM_ENTITY, "5" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENTT, { NUM_ENTITY, "5" }, { NUM_ENTITY, "1" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent* between synonym and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { PARENTT, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "2", "4" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { PARENTT, { WILDCARD, "_" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent* between entity and wildcard") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { PARENTT, { NUM_ENTITY, "2" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "1", "2", "3", "4", "5", "6" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { PARENTT, { WILDCARD, "_" }, { NUM_ENTITY, "1" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Parent*(_, _)") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { PARENTT, { WILDCARD, "_" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6" }));
}

TEST_CASE("Test evaluation of Parent* with invalid arguments") {
    PKBMock pkb(1);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } },
                          std::vector<std::string>(),
                          { { PARENTT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonym
    Query query_proc = { { { "s1", STMT }, { "s2", PROCEDURE } },
                         { "s1" },
                         { { PARENTT, { STMT_SYNONYM, "s1" }, { PROC_SYNONYM, "s2" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(query_proc).empty());

    Query query_const = {
        { { "s", CONSTANT } }, { "s" }, { { PARENTT, { NUM_ENTITY, "1" }, { CONST_SYNONYM, "s" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    Query query_var = {
        { { "s", VARIABLE } }, { "s" }, { { PARENTT, { VAR_SYNONYM, "s" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_var).empty());

    Query query = {
        { { "s", STMT } }, { "s" }, { { PARENTT, { NAME_ENTITY, "name1" }, { NAME_ENTITY, "name2" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Uses between synonyms") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesStmt = { { { "s", STMT }, { "v", VARIABLE } },
                            { "s" },
                            { { USES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                            {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesStmt), { "1", "2", "4", "5", "6" }));
    Query queryUsesVar = { { { "s", STMT }, { "v", VARIABLE } },
                           { "v" },
                           { { USES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                           {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesVar), { "x", "z", "m", "n", "count" }));

    Query queryUsepProc = { { { "p", PROCEDURE }, { "v", VARIABLE } },
                            { "p" },
                            { { USES, { PROC_SYNONYM, "p" }, { VAR_SYNONYM, "v" } } },
                            {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepProc), { "foo", "bar" }));
    Query queryUsepVar = { { { "p", PROCEDURE }, { "v", VARIABLE } },
                           { "v" },
                           { { USES, { PROC_SYNONYM, "p" }, { VAR_SYNONYM, "v" } } },
                           {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepVar), { "x", "z", "m", "n", "count" }));
}

TEST_CASE("Test evaluation of Uses between entity and synonym") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesStmt = {
        { { "s", STMT } }, { "s" }, { { USES, { STMT_SYNONYM, "s" }, { NAME_ENTITY, "n" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesStmt), { "2", "4", "5" }));
    Query queryUsesVar = {
        { { "v", VARIABLE } }, { "v" }, { { USES, { NUM_ENTITY, "2" }, { VAR_SYNONYM, "v" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesVar), { "z", "m", "n", "count" }));

    Query queryUsepProc = {
        { { "p", PROCEDURE } }, { "p" }, { { USES, { PROC_SYNONYM, "p" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepProc), { "foo" }));
    Query queryUsepVar = {
        { { "v", VARIABLE } }, { "v" }, { { USES, { NAME_ENTITY, "bar" }, { VAR_SYNONYM, "v" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepVar), { "z", "m", "n", "count" }));
}

TEST_CASE("Test evaluation of Uses between entities") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesTrue = { { { "s", STMT } }, { "s" }, { { USES, { NUM_ENTITY, "5" }, { NAME_ENTITY, "n" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsesFalse = {
        { { "s", STMT } }, { "s" }, { { USES, { NUM_ENTITY, "5" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryUsesFalse).empty());

    Query queryUsepTrue = {
        { { "s", STMT } }, { "s" }, { { USES, { NAME_ENTITY, "foo" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsepFalse = {
        { { "s", STMT } }, { "s" }, { { USES, { NAME_ENTITY, "bar" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryUsepFalse).empty());
}

TEST_CASE("Test evaluation of Uses between synonym and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesS = { { { "s", STMT } }, { "s" }, { { USES, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesS), { "1", "2", "4", "5", "6" }));

    Query queryUseP = { { { "p", PROCEDURE } }, { "p" }, { { USES, { PROC_SYNONYM, "p" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUseP), { "foo", "bar" }));
}

TEST_CASE("Test evaluation of Uses between entity and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesTrue = { { { "s", STMT } }, { "s" }, { { USES, { NUM_ENTITY, "5" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsesFalse = { { { "s", STMT } }, { "s" }, { { USES, { NUM_ENTITY, "3" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryUsesFalse).empty());

    Query queryUsepTrue = { { { "s", STMT } }, { "s" }, { { USES, { NAME_ENTITY, "foo" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsepTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsepFalse = {
        { { "s", STMT } }, { "s" }, { { USES, { NAME_ENTITY, "main" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryUsepFalse).empty());
}

TEST_CASE("Test evaluation of Uses with invalid arguments") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query queryEmpty = { { { "s", STMT }, { "V", VARIABLE } },
                         std::vector<std::string>(),
                         { { USES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "V" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(queryEmpty).empty());

    // invalid synonym
    Query queryProc = { { { "s1", STMT }, { "s2", STMT } },
                        { "s1" },
                        { { USES, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(qe.evaluateQuery(queryProc).empty());

    Query queryConst = {
        { { "c", CONSTANT } }, { "c" }, { { USES, { CONST_SYNONYM, "c" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryConst).empty());

    // invalid entity
    Query queryE1 = { { { "s", STMT } }, { "s" }, { { USES, { NAME_ENTITY, "1" }, { NAME_ENTITY, "x" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryE1).empty());
    Query queryE2 = { { { "s", STMT } }, { "s" }, { { USES, { NUM_ENTITY, "1" }, { STMT_SYNONYM, "x" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryE2).empty());

    // invalid wildcard
    Query queryE3 = { { { "s", STMT } }, { "s" }, { { USES, { WILDCARD, "_" }, { NAME_ENTITY, "x" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryE3).empty());
}

TEST_CASE("Test evaluation of Modifies between synonyms") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsStmt = { { { "s", STMT }, { "v", VARIABLE } },
                            { "s" },
                            { { MODIFIES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                            {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsStmt),
                                       { "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryUsesVar = { { { "s", STMT }, { "v", VARIABLE } },
                           { "v" },
                           { { MODIFIES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                           {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesVar), { "a", "n", "random", "y" }));

    Query queryModpProc = { { { "p", PROCEDURE }, { "v", VARIABLE } },
                            { "p" },
                            { { MODIFIES, { PROC_SYNONYM, "p" }, { VAR_SYNONYM, "v" } } },
                            {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpProc), { "foo", "bar" }));

    Query queryModIf = { { { "ifs", IF }, { "v", VARIABLE } },
                         { "ifs" },
                         { { MODIFIES, { STMT_SYNONYM, "ifs" }, { VAR_SYNONYM, "v" } } },
                         {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModIf), { "4" }));
    Query queryModWhile = { { { "w", WHILE }, { "v", VARIABLE } },
                            { "w" },
                            { { MODIFIES, { STMT_SYNONYM, "w" }, { VAR_SYNONYM, "v" } } },
                            {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModWhile), { "7", "8" }));

    Query queryModpVar = { { { "p", PROCEDURE }, { "v", VARIABLE } },
                           { "v" },
                           { { MODIFIES, { PROC_SYNONYM, "p" }, { VAR_SYNONYM, "v" } } },
                           {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpVar), { "a", "n", "random", "y" }));
}

TEST_CASE("Test evaluation of Modifies between entity and synonym") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsStmt = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { STMT_SYNONYM, "s" }, { NAME_ENTITY, "n" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsStmt), { "2", "4", "5" }));
    Query queryModsVar = {
        { { "v", VARIABLE } }, { "v" }, { { MODIFIES, { NUM_ENTITY, "2" }, { VAR_SYNONYM, "v" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsVar), { "a", "n", "random", "y" }));

    Query queryModpProc = {
        { { "p", PROCEDURE } }, { "p" }, { { MODIFIES, { PROC_SYNONYM, "p" }, { NAME_ENTITY, "y" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpProc), { "foo", "bar" }));
    Query queryModpVar = {
        { { "v", VARIABLE } }, { "v" }, { { MODIFIES, { NAME_ENTITY, "bar" }, { VAR_SYNONYM, "v" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpVar), { "a", "n", "random", "y" }));
}

TEST_CASE("Test evaluation of Modifies between entities") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsTrue = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NUM_ENTITY, "5" }, { NAME_ENTITY, "n" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModsFalse = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NUM_ENTITY, "5" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryModsFalse).empty());

    Query queryModpTrue = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NAME_ENTITY, "foo" }, { NAME_ENTITY, "y" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModpFalse = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NAME_ENTITY, "foo" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryModpFalse).empty());
}

TEST_CASE("Test evaluation of Modifies between synonym and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUsesS = { { { "s", STMT } }, { "s" }, { { MODIFIES, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUsesS), { "2", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryUseP = {
        { { "p", PROCEDURE } }, { "p" }, { { MODIFIES, { PROC_SYNONYM, "p" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUseP), { "foo", "bar" }));
}

TEST_CASE("Test evaluation of Modifies between entity and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryModsTrue = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NUM_ENTITY, "5" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModsTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModsFalse = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NUM_ENTITY, "1" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryModsFalse).empty());

    Query queryModpTrue = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NAME_ENTITY, "foo" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryModpTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
    Query queryModpFalse = {
        { { "s", STMT } }, { "s" }, { { MODIFIES, { NAME_ENTITY, "main" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryModpFalse).empty());
}

TEST_CASE("Test evaluation of Modifies with invalid arguments") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query queryEmpty = { { { "s", STMT }, { "V", VARIABLE } },
                         std::vector<std::string>(),
                         { { MODIFIES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "V" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(queryEmpty).empty());

    // invalid synonym
    Query queryProc = { { { "s1", STMT }, { "s2", STMT } },
                        { "s1" },
                        { { MODIFIES, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(qe.evaluateQuery(queryProc).empty());

    Query queryConst = {
        { { "c", CONSTANT } }, { "c" }, { { MODIFIES, { CONST_SYNONYM, "c" }, { NAME_ENTITY, "x" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryConst).empty());

    // invalid entity
    Query queryE1 = { { { "s", STMT } }, { "s" }, { { MODIFIES, { NAME_ENTITY, "1" }, { NAME_ENTITY, "x" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryE1).empty());
    Query queryE2 = { { { "s", STMT } }, { "s" }, { { MODIFIES, { NUM_ENTITY, "1" }, { STMT_SYNONYM, "x" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryE2).empty());

    // invalid wildcard
    Query queryE3 = { { { "s", STMT } }, { "s" }, { { MODIFIES, { WILDCARD, "_" }, { NAME_ENTITY, "x" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryE3).empty());
}

TEST_CASE("Test combined Uses and Modifies") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryUse = { { { "s", STMT }, { "v", VARIABLE } },
                       { "s" },
                       { { USES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryUse), { "1", "2", "4", "5", "6" }));

    Query queryMod = { { { "s", STMT }, { "v", VARIABLE } },
                       { "s" },
                       { { MODIFIES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryMod), { "2", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryDouble = { { { "s", STMT }, { "v", VARIABLE } },
                          { "s" },
                          { { MODIFIES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } },
                            { USES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v" } } },
                          {} };
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

    Query query1 = { { { "a", ASSIGN } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "0" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "1", "2", "3" }));

    Query query2 = { { { "a", ASSIGN }, { "cl", CALL } },
                     { "cl" },
                     {},
                     { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "cenX" }, "cenX+x" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "4", "9" }));

    Query query3 = { { { "a", ASSIGN }, { "v", VARIABLE } },
                     { "v" },
                     {},
                     { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "cenX+x" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX" }));
}

TEST_CASE("Test pattern of subexpression") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "count" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "6", "12", "13" }));

    Query query2 = { { { "a", ASSIGN }, { "cl", CALL } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "count" }, "0" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "1" }));

    Query query3 = { { { "a", ASSIGN }, { "v", VARIABLE } },
                     { "v" },
                     {},
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "0" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX", "cenY", "count" }));
}

TEST_CASE("Test pattern of wildcard pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1),
                                       { "1", "2", "3", "6", "7", "8", "11", "12", "13", "14" }));

    Query query2 = { { { "a", ASSIGN } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "cenX" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "2", "7", "12" }));

    Query query3 = { { { "a", ASSIGN }, { "v", VARIABLE } },
                     { "v" },
                     {},
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX", "cenY", "count", "normSq", "flag" }));
}

TEST_CASE("Test invalid pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    // assignment should be a synonym
    Query query1 = { { { "v", VARIABLE } },
                     { "v" },
                     {},
                     { { ASSIGN_PATTERN_WILDCARD, { NUM_ENTITY, "1" }, { VAR_SYNONYM, "v" }, "_" } } };
    REQUIRE(qe.evaluateQuery(query1).empty());

    // 1st argument should be a synonym, name-entity or wildcard
    Query query2 = { { { "a", ASSIGN }, { "p", PROCEDURE } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a" }, { PROC_SYNONYM, "p" }, "0" } } };
    REQUIRE(qe.evaluateQuery(query2).empty());

    // invalid pattern
    Query query3 = { { { "a", ASSIGN } },
                     { "a" },
                     {},
                     { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "\"0\"" } } };
    REQUIRE(qe.evaluateQuery(query3).empty());
}

TEST_CASE("Test Follows/Follows* and Pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "s", STMT }, { "a", ASSIGN } },
                     { "s" },
                     { { FOLLOWST, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "cenY" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "1", "2", "6", "7", "12" }));

    Query query2 = { { { "a", ASSIGN } },
                     { "a" },
                     { { FOLLOWST, { WILDCARD, "_" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "count" }, "_" } } };
    REQUIRE(qe.evaluateQuery(query2).empty());

    Query query3 = { { { "v", VARIABLE }, { "a", ASSIGN } },
                     { "v" },
                     { { FOLLOWS, { NUM_ENTITY, "6" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "cenX" }));
}

TEST_CASE("Test Parent/Parent* and Pattern") {
    PKBMock pkb(0);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "s", STMT }, { "a", ASSIGN } },
                     { "a" },
                     { { PARENTT, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "cenX" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "7", "12" }));

    Query query2 = { { { "a", ASSIGN }, { "w", WHILE } },
                     { "a" },
                     { { PARENT, { STMT_SYNONYM, "w" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "count" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "6" }));

    Query query3 = { { { "ifs", IF }, { "v", VARIABLE }, { "a", ASSIGN } },
                     { "v" },
                     { { PARENT, { STMT_SYNONYM, "ifs" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "flag", "cenX", "cenY" }));
}

TEST_CASE("Test Uses and Pattern") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN }, { "v", VARIABLE } },
                     { "a" },
                     { { USES, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "5" }));

    Query query2 = { { { "a", ASSIGN }, { "ifs", IF }, { "v", VARIABLE } },
                     { "a" },
                     { { USES, { STMT_SYNONYM, "ifs" }, { VAR_SYNONYM, "v" } } },
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "1" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "5" }));

    Query query3 = { { { "a", ASSIGN } },
                     { "a" },
                     { { USES, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "m" } } },
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "z" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "6" }));
}

TEST_CASE("Test Modifies and Pattern") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query1 = { { { "a", ASSIGN } },
                     { "a" },
                     { { MODIFIES, { STMT_SYNONYM, "a" }, { WILDCARD, "_" } } },
                     { { ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { NAME_ENTITY, "y" }, "_" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "6" }));

    Query query2 = { { { "a", ASSIGN }, { "ifs", IF }, { "v", VARIABLE } },
                     { "a" },
                     { { MODIFIES, { STMT_SYNONYM, "ifs" }, { VAR_SYNONYM, "v" } } },
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "1" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "5" }));
}

TEST_CASE("multiple relation clauses") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    // stmt s1; stmt s2; variable v; Select s2 such that Parent(s2, s1) and Modifies(s1, v) and Uses(s1, v)
    Query query1 = { { { "s1", STMT }, { "s2", STMT }, { "v", VARIABLE } },
                     { "s2" },
                     { { MODIFIES, { STMT_SYNONYM, "s1" }, { VAR_SYNONYM, "v" } },
                       { USES, { STMT_SYNONYM, "s1" }, { VAR_SYNONYM, "v" } },
                       { PARENT, { STMT_SYNONYM, "s2" }, { STMT_SYNONYM, "s1" } } },
                     {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query1), { "4" }));

    // stmt s1; stmt s2; variable v; Select v such that Uses(s1, "x") and Follows*(s1, s2) and Modifies(s2, v)
    Query query2 = { { { "s1", STMT }, { "s2", STMT }, { "v", VARIABLE } },
                     { "v" },
                     { { MODIFIES, { STMT_SYNONYM, "s2" }, { VAR_SYNONYM, "v" } },
                       { USES, { STMT_SYNONYM, "s1" }, { NAME_ENTITY, "x" } },
                       { FOLLOWST, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                     {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query2), { "random", "n", "y", "a" }));

    // assign a; stmt s; variable v; variable v2; Select v2 such that Parent*(s, a) and Modifies(s, v2) and Uses(a, v) pattern a(v, _"1"_)
    Query query3 = { { { "s", STMT }, { "a", ASSIGN }, { "v", VARIABLE }, { "v2", VARIABLE } },
                     { "v2" },
                     { { MODIFIES, { STMT_SYNONYM, "s" }, { VAR_SYNONYM, "v2" } },
                       { USES, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" } },
                       { PARENTT, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "a" } } },
                     { { ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { VAR_SYNONYM, "v" }, "1" } } };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query3), { "n", "y" }));
}

TEST_CASE("Test evaluation of Next or Next* between synonyms") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s1", STMT }, { "s2", STMT } },
                       { "s1" },
                       { { NEXT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryPost = { { { "s1", STMT }, { "s2", STMT } },
                        { "s2" },
                        { { NEXT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "4", "5", "6", "7", "8", "9" }));

    Query querySelf = { { { "s", STMT } }, { "s" }, { { NEXT, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(qe.evaluateQuery(querySelf).empty());

    Query querySelfTransitive = {
        { { "s", STMT } }, { "s" }, { { NEXTT, { STMT_SYNONYM, "s" }, { STMT_SYNONYM, "s" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(querySelfTransitive), { "7", "8", "9" }));

    Query queryNonStmt = { { { "rd", READ }, { "w", WHILE } },
                           { "w" },
                           { { NEXTT, { STMT_SYNONYM, "rd" }, { STMT_SYNONYM, "w" } } },
                           {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryNonStmt), { "7", "8" }));
}

TEST_CASE("Test evaluation of Next or Next* between entity and synonym") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { NEXT, { NUM_ENTITY, "4" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "5", "6" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { NEXTT, { STMT_SYNONYM, "s" }, { NUM_ENTITY, "7" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "3", "4", "5", "6", "7", "8", "9" }));
}

TEST_CASE("Test evaluation of Next or Next* between entities") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { NEXT, { NUM_ENTITY, "7" }, { NUM_ENTITY, "7" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { NEXTT, { NUM_ENTITY, "9" }, { NUM_ENTITY, "7" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
}

TEST_CASE("Test evaluation of Next or Next* between synonym and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "s", STMT } }, { "s" }, { { NEXT, { STMT_SYNONYM, "s" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "1", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryPost = { { { "s", STMT } }, { "s" }, { { NEXTT, { WILDCARD, "_" }, { STMT_SYNONYM, "s" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "2", "4", "5", "6", "7", "8", "9" }));
}

TEST_CASE("Test evaluation of Next/Next* between entity and wildcard") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = { { { "s", STMT } }, { "s" }, { { NEXT, { NUM_ENTITY, "9" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue),
                                       { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));

    Query queryFalse = { { { "s", STMT } }, { "s" }, { { NEXTT, { WILDCARD, "_" }, { NUM_ENTITY, "3" } } }, {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Next or Next*(_, _)") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "s", STMT } }, { "s" }, { { NEXT, { WILDCARD, "_" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "1", "2", "3", "4", "5", "6", "7", "8", "9" }));
}


TEST_CASE("Test evaluation of Next or Next* with invalid arguments") {
    PKBMock pkb(2);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "s1", STMT }, { "s2", STMT } },
                          std::vector<std::string>(),
                          { { NEXT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonyms
    Query query_var = { { { "v", VARIABLE }, { "s", STMT } },
                        { "s" },
                        { { NEXTT, { VAR_SYNONYM, "v" }, { STMT_SYNONYM, "s" } } },
                        {} };
    REQUIRE(qe.evaluateQuery(query_var).empty());
    Query query_const = { { { "c", CONSTANT }, { "s", STMT } },
                          { "s" },
                          { { NEXTT, { VAR_SYNONYM, "c" }, { STMT_SYNONYM, "s" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_const).empty());

    // invalid entity
    Query query = { { { "s", STMT } }, { "s" }, { { NEXT, { VAR_SYNONYM, "name1" }, { VAR_SYNONYM, "name2" } } }, {} };
    REQUIRE(qe.evaluateQuery(query).empty());
}

TEST_CASE("Test evaluation of Calls or Calls* between synonyms") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "p1", PROCEDURE }, { "p2", PROCEDURE } },
                       { "p1" },
                       { { CALLS, { PROC_SYNONYM, "p1" }, { PROC_SYNONYM, "p2" } } },
                       {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "first", "second" }));

    Query queryPost = { { { "p1", PROCEDURE }, { "p2", PROCEDURE } },
                        { "p2" },
                        { { CALLST, { PROC_SYNONYM, "p1" }, { PROC_SYNONYM, "p2" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "second", "third" }));

    Query querySelf = {
        { { "p", PROCEDURE } }, { "p" }, { { CALLST, { PROC_SYNONYM, "p" }, { PROC_SYNONYM, "p" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(querySelf).empty());
}

TEST_CASE("Test evaluation of Calls or Calls* between entity and synonym") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = {
        { { "p", PROCEDURE } }, { "p" }, { { CALLS, { NAME_ENTITY, "first" }, { PROC_SYNONYM, "p" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "second" }));

    Query queryPost = { { { "p", PROCEDURE } },
                        { "p" },
                        { { CALLST, { PROC_SYNONYM, "p" }, { NAME_ENTITY, "third" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "first", "second" }));
}

TEST_CASE("Test evaluation of Calls or Calls* between entities") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryFalse = { { { "p", PROCEDURE } },
                         { "p" },
                         { { CALLS, { NAME_ENTITY, "first" }, { NAME_ENTITY, "third" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());

    Query queryTrue = { { { "p", PROCEDURE } },
                        { "p" },
                        { { CALLST, { NAME_ENTITY, "first" }, { NAME_ENTITY, "second" } } },
                        {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "first", "second", "third" }));
}

TEST_CASE("Test evaluation of Calls or Calls* between synonym and wildcard") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryPre = { { { "p", PROCEDURE } }, { "p" }, { { CALLS, { PROC_SYNONYM, "p" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPre), { "first", "second" }));

    Query queryPost = {
        { { "p", PROCEDURE } }, { "p" }, { { CALLST, { WILDCARD, "_" }, { PROC_SYNONYM, "p" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryPost), { "second", "third" }));
}

TEST_CASE("Test evaluation of Calls/Calls* between entity and wildcard") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query queryTrue = {
        { { "p", PROCEDURE } }, { "p" }, { { CALLS, { NAME_ENTITY, "first" }, { WILDCARD, "_" } } }, {}
    };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(queryTrue), { "first", "second", "third" }));

    Query queryFalse = {
        { { "p", PROCEDURE } }, { "p" }, { { CALLST, { WILDCARD, "_" }, { NAME_ENTITY, "first" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(queryFalse).empty());
}

TEST_CASE("Test evaluation of Calls or Calls*(_, _)") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);
    Query query = { { { "p", PROCEDURE } }, { "p" }, { { CALLST, { WILDCARD, "_" }, { WILDCARD, "_" } } }, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query), { "first", "second", "third" }));
}

TEST_CASE("Test evaluation of Calls or Calls* with invalid arguments") {
    PKBMock pkb(4);
    queryevaluator::QueryEvaluator qe(&pkb);

    // invalid query
    Query query_empty = { { { "p1", PROCEDURE }, { "p2", PROCEDURE } },
                          std::vector<std::string>(),
                          { { CALLS, { PROC_SYNONYM, "p1" }, { PROC_SYNONYM, "p2" } } },
                          {} };
    REQUIRE(qe.evaluateQuery(query_empty).empty());

    // invalid synonyms
    Query query_stmt = { { { "v", VARIABLE }, { "p", PROCEDURE } },
                         { "v" },
                         { { CALLST, { VAR_SYNONYM, "v" }, { PROC_SYNONYM, "p" } } },
                         {} };
    REQUIRE(qe.evaluateQuery(query_stmt).empty());

    // invalid entity
    Query query_num = {
        { { "p", PROCEDURE } }, { "p" }, { { CALLST, { NUM_ENTITY, "2" }, { PROC_SYNONYM, "p" } } }, {}
    };
    REQUIRE(qe.evaluateQuery(query_num).empty());
}

TEST_CASE("Test getting attributes") {
    PKBMock pkb(3);
    queryevaluator::QueryEvaluator qe(&pkb);

    Query query_proc_name = { { { "cl", CALL } }, { { CALL_PROC_NAME, "cl" } }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query_proc_name),
                                       { "computeCentroid", "printResults", "readPoint" }));

    Query query_read_var = { { { "rd", READ } }, { { READ_VAR_NAME, "rd" } }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query_read_var), { "x", "y" }));

    Query query_print_var = { { { "pt", PRINT } }, { { PRINT_VAR_NAME, "pt" } }, {}, {} };
    REQUIRE(checkIfVectorOfStringMatch(qe.evaluateQuery(query_print_var), { "flag", "cenX", "cenY", "normSq" }));

    // test invalid attributes
    Query query_invalid = { { { "rd", STMT } }, { { READ_VAR_NAME, "rd" } }, {}, {} };
    REQUIRE(qe.evaluateQuery(query_invalid).empty());
}

} // namespace qetest
} // namespace qpbackend
