#include "Foost.hpp"
#include "Optimisation.h"
#include "QEHelper.h"
#include "QPTypes.h"
#include "TestQEHelper.h"
#include "catch.hpp"

namespace qpbackend {
namespace optimisationtest {
CLAUSE_LIST getClauses(const Query& q) {
    CLAUSE_LIST result;
    for (auto relationClause : q.suchThatClauses) {
        CLAUSE clause = { std::get<0>(relationClause), std::get<1>(relationClause),
                          std::get<2>(relationClause), "" };
        result.push_back(clause);
    }
    result.insert(result.end(), q.patternClauses.begin(), q.patternClauses.end());
    return result;
}


TEST_CASE("Test only entity-entity clauses") {
    Query q = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "7" }, { NUM_ENTITY, "8" } } }, {} };
    CLAUSE_LIST clauses = getClauses(q);

    std::vector<CLAUSE_LIST> group1 = { { { FOLLOWS, { NUM_ENTITY, "7" }, { NUM_ENTITY, "8" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };

    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test only entity-synonym clauses") {
    Query q = { { { "s", STMT } }, { "s" }, { { FOLLOWS, { NUM_ENTITY, "7" }, { STMT_SYNONYM, "a" } } }, {} };

    std::vector<CLAUSE_LIST> group1 = { { { FOLLOWS, { NUM_ENTITY, "7" }, { STMT_SYNONYM, "a" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };

    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test only synonym-entity clauses") {
    Query q = { { { "a", STMT } }, { "a" }, { { FOLLOWS, { STMT_SYNONYM, "a" }, { NUM_ENTITY, "7" } } }, {} };
    std::vector<CLAUSE_LIST> group1 = { { { FOLLOWS, { STMT_SYNONYM, "a" }, { NUM_ENTITY, "7" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test only patterns synonym-entity clauses") {
    Query q = { { { "a1", STMT } },
                { "a1" },
                {},
                {
                { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a1" }, { NAME_ENTITY, "v" }, "_\"x+y\"_" },
                } };
    std::vector<CLAUSE_LIST> group1 = {
        { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a1" }, { NAME_ENTITY, "v" }, "_\"x+y\"_" } }
    };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test only WITH entity-entity clauses") {
    Query q = { {}, RETURN_CANDIDATE_LIST(), { { WITH, { NUM_ENTITY, "1" }, { NUM_ENTITY, "1" } } }, {} };
    std::vector<CLAUSE_LIST> group1 = { { { WITH, { NUM_ENTITY, "1" }, { NUM_ENTITY, "1" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test only WITH entity-synonym clauses") {
    Query q = { { { "s", STMT } }, { "s" }, { { WITH, { NUM_ENTITY, "1" }, { STMT_SYNONYM, "s" } } }, {} };
    std::vector<CLAUSE_LIST> group1 = { { { WITH, { NUM_ENTITY, "1" }, { STMT_SYNONYM, "s" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}


TEST_CASE("Test only synonym-synonym clauses") {
    Query q = { { { "s1", STMT }, { "s2", STMT } },
                { "s1" },
                { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                {} };
    std::vector<CLAUSE_LIST> group1 = { { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test two synonym-entity clauses") {
    Query q = { { { "s1", STMT } },
                { "s1" },
                { { FOLLOWS, { STMT_SYNONYM, "s1" }, { WILDCARD, "_" } },
                  { USES, { STMT_SYNONYM, "s1" }, { WILDCARD, "_" } } },
                {} };
    std::vector<CLAUSE_LIST> group1 = { { { FOLLOWS, { STMT_SYNONYM, "s1" }, { WILDCARD, "_" }, "" } },
                                        { { USES, { STMT_SYNONYM, "s1" }, { WILDCARD, "_" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test two synonym-synonym clauses") {
    Query q = { { { "s1", STMT }, { "s2", STMT } },
                { "s1" },
                { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } },
                  { USES, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } } },
                {} };
    std::vector<CLAUSE_LIST> group1 = {
        { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" }, "" },
          { USES, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" }, "" } },
    };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test two independent synonym-synonym clauses") {
    Query q = { { { "s1", STMT }, { "s2", STMT }, { "s3", STMT }, { "s4", STMT } },
                { "s1" },
                { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } },
                  { USES, { STMT_SYNONYM, "s3" }, { STMT_SYNONYM, "s4" } } },
                {} };
    std::vector<CLAUSE_LIST> group1 = { { { USES, { STMT_SYNONYM, "s3" }, { STMT_SYNONYM, "s4" }, "" } } };
    std::vector<CLAUSE_LIST> group2 = { { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1, group2 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

TEST_CASE("Test self-loop synonym-synonym clauses") {
    Query q = { { { "p", PROCEDURE } }, { "p" }, { { CALLST, { PROC_SYNONYM, "p" }, { PROC_SYNONYM, "p" } } }, {} };
    std::vector<CLAUSE_LIST> group1 = { { { CALLST, { PROC_SYNONYM, "p" }, { PROC_SYNONYM, "p" }, "" } } };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1 };
    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);
    REQUIRE(actual == expected);
}

// This test is the same as in the design doc.
TEST_CASE("Test complex clauses") {
    Query q = { { { "a1", ASSIGN },
                  { "a2", ASSIGN },
                  { "a3", ASSIGN },
                  { "s1", STMT },
                  { "s2", STMT },
                  { "s3", STMT },
                  { "s4", STMT },
                  { "s5", STMT },
                  { "s6", STMT },
                  { "v1", VARIABLE },
                  { "v2", VARIABLE } },
                { "s1", "s2", "v2" },
                {
                { FOLLOWS, { NUM_ENTITY, "3" }, { NUM_ENTITY, "7" } },
                { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s3" } },
                { USES, { STMT_SYNONYM, "s3" }, { STMT_SYNONYM, "s1" } },
                { NEXT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "a1" } },
                { PARENT, { NUM_ENTITY, "3" }, { STMT_SYNONYM, "s2" } },
                { NEXTT, { STMT_SYNONYM, "s3" }, { STMT_SYNONYM, "s1" } },
                { WITH, { STMT_SYNONYM, "s2" }, { NUM_ENTITY, "7" } },
                { PARENT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" } },
                { FOLLOWS, { STMT_SYNONYM, "s3" }, { NUM_ENTITY, "7" } },
                { WITH, { NUM_ENTITY, "1" }, { NUM_ENTITY, "1" } },
                { USES, { STMT_SYNONYM, "s1" }, { VAR_SYNONYM, "v1" } },
                { FOLLOWST, { STMT_SYNONYM, "s5" }, { STMT_SYNONYM, "a3" } },
                { USES, { STMT_SYNONYM, "a2" }, { VAR_SYNONYM, "v2" } },
                { MODIFIES, { STMT_SYNONYM, "s4" }, { VAR_SYNONYM, "v2" } },
                { FOLLOWST, { STMT_SYNONYM, "s5" }, { STMT_SYNONYM, "s6" } },
                },
                {
                { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a1" }, { NAME_ENTITY, "v" }, "_\"x+y\"_" },
                } };
    std::vector<CLAUSE_LIST> group1 = { { { FOLLOWS, { NUM_ENTITY, "3" }, { NUM_ENTITY, "7" }, "" } } };
    std::vector<CLAUSE_LIST> group2 = { { { WITH, { NUM_ENTITY, "1" }, { NUM_ENTITY, "1" }, "" } } };
    std::vector<CLAUSE_LIST> group3 = { { { FOLLOWST, { STMT_SYNONYM, "s5" }, { STMT_SYNONYM, "s6" }, "" } },
                                        { { FOLLOWST, { STMT_SYNONYM, "s5" }, { STMT_SYNONYM, "a3" }, "" } } };
    std::vector<CLAUSE_LIST> group4 = { { { USES, { STMT_SYNONYM, "a2" }, { VAR_SYNONYM, "v2" }, "" } },
                                        { { MODIFIES, { STMT_SYNONYM, "s4" }, { VAR_SYNONYM, "v2" }, "" } } };
    std::vector<CLAUSE_LIST> group5 = {
        { { WITH, { STMT_SYNONYM, "s2" }, { NUM_ENTITY, "7" }, "" } },
        { { PARENT, { NUM_ENTITY, "3" }, { STMT_SYNONYM, "s2" }, "" } },
        { { PARENT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s2" }, "" } },
        { { FOLLOWS, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "s3" }, "" },
          { USES, { STMT_SYNONYM, "s3" }, { STMT_SYNONYM, "s1" }, "" },
          { NEXTT, { STMT_SYNONYM, "s3" }, { STMT_SYNONYM, "s1" }, "" } },
        { { FOLLOWS, { STMT_SYNONYM, "s3" }, { NUM_ENTITY, "7" }, "" } },
        { { USES, { STMT_SYNONYM, "s1" }, { VAR_SYNONYM, "v1" }, "" } },
        { { NEXT, { STMT_SYNONYM, "s1" }, { STMT_SYNONYM, "a1" }, "" } },
        { { ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "a1" }, { NAME_ENTITY, "v" }, "_\"x+y\"_" } },
    };
    std::vector<std::vector<CLAUSE_LIST>> expected = { group1, group2, group3, group4, group5 };

    std::vector<std::vector<CLAUSE_LIST>> actual =
    optimisation::optimizeQueries(getClauses(q), q.returnCandidates);

    REQUIRE(actual == expected);
}

} // namespace optimisationtest
} // namespace qpbackend
