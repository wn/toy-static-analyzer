#include "QPTypes.h"
#include "Query.h"
#include "catch.hpp"

namespace qpbackend {

TEST_CASE("Test default constructor produces empty query") {
    Query query = Query();

    REQUIRE(query.declarationMap.empty());
    REQUIRE(query.returnCandidates.empty());
    REQUIRE(query.patternClauses.empty());
    REQUIRE(query.suchThatClauses.empty());
}

TEST_CASE("Test empty query equality") {
    REQUIRE(Query() == Query());
}

TEST_CASE("Test query equality") {
    REQUIRE(
    Query(/* declarationMap */ { { "anyVariable", EntityType::VARIABLE }, { "procedure", EntityType::PROCEDURE } },
          /* returnCandidates */ std::vector<std::string>({ "anyVariable", "procedure" }),
          /* suchThatClauses */
          {
          RELATIONTUPLE{ ClauseType::FOLLOWS, { NUM_ENTITY, "1" }, { NUM_ENTITY, "2" } },
          RELATIONTUPLE{ ClauseType::MODIFIES, { PROC_SYNONYM, "procedure" }, { VAR_SYNONYM, "anyVariable" } },
          },
          /* patternClauses */
          { PATTERNTUPLE{ ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { STMT_SYNONYM, "a" }, "_" },
            PATTERNTUPLE{ ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "a+s*l/d+k-j" },
            PATTERNTUPLE{ ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "testingisgood" }, { NAME_ENTITY, "greedisgood" }, "(expressionInBrackets)" } }) ==
    Query(/* declarationMap */ { { "anyVariable", EntityType::VARIABLE }, { "procedure", EntityType::PROCEDURE } },
          /* returnCandidates */ std::vector<std::string>({ "anyVariable", "procedure" }),
          /* suchThatClauses */
          {
          RELATIONTUPLE{ ClauseType::FOLLOWS, { NUM_ENTITY, "1" }, { NUM_ENTITY, "2" } },
          RELATIONTUPLE{ ClauseType::MODIFIES, { PROC_SYNONYM, "procedure" }, { VAR_SYNONYM, "anyVariable" } },
          },
          /* patternClauses */
          { PATTERNTUPLE{ ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { STMT_SYNONYM, "a" }, "_" },
            PATTERNTUPLE{ ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "a+s*l/d+k-j" },
            PATTERNTUPLE{ ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "testingisgood" }, { NAME_ENTITY, "greedisgood" }, "(expressionInBrackets)" } }));
}

TEST_CASE("Test query equality failure") {
    REQUIRE_FALSE(
    Query(/* declarationMap */ { { "anyVariable", EntityType::VARIABLE }, { "procedure", EntityType::PROCEDURE } },
          /* returnCandidates */ std::vector<std::string>({ "anyVariable", "procedure" }),
          /* suchThatClauses */
          {
          RELATIONTUPLE{ ClauseType::FOLLOWS, { NUM_ENTITY, "1" }, { NUM_ENTITY, "2" } },
          RELATIONTUPLE{ ClauseType::MODIFIES, { PROC_SYNONYM, "procedure" }, { VAR_SYNONYM, "anyVariable" } },
          },
          /* patternClauses */
          { PATTERNTUPLE{ ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { STMT_SYNONYM, "a" }, "_" },
            PATTERNTUPLE{ ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "a+s*l/d+k-j" },
            PATTERNTUPLE{ ASSIGN_PATTERN_EXACT, { STMT_SYNONYM, "testingisgood" }, { NAME_ENTITY, "greedisgood" }, "(expressionInBrackets)" } }) ==
    Query({ /* declarationMap */ { "anyVariable", EntityType::VARIABLE } },
          /* returnCandidates */ std::vector<std::string>({ "anyVariable" }),
          /* suchThatClauses */ {},
          /* patternClauses */
          {
          PATTERNTUPLE{ ASSIGN_PATTERN_WILDCARD, { STMT_SYNONYM, "a" }, { STMT_SYNONYM, "a" }, "_" },
          PATTERNTUPLE{ ASSIGN_PATTERN_SUB_EXPR, { STMT_SYNONYM, "a" }, { WILDCARD, "_" }, "a+s*l/d+k-j" },
          }));
}


} // namespace qpbackend
