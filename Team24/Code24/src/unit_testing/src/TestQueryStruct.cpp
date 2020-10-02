#include "QPTypes.h"
#include "Query.h"
#include "catch.hpp"

namespace qpbackend {

TEST_CASE("Test default constructor produces empty query") {
    Query query = Query();

    REQUIRE(query.declarationMap.empty());
    REQUIRE(query.synonymsToReturn.empty());
    REQUIRE(query.patternClauses.empty());
    REQUIRE(query.suchThatClauses.empty());
}

TEST_CASE("Test empty query equality") {
    REQUIRE(Query() == Query());
}

TEST_CASE("Test query equality") {
    REQUIRE(
    Query(/* declarationMap */ { { "anyVariable", EntityType::VARIABLE }, { "procedure", EntityType::PROCEDURE } },
          /* synonymsToReturn */ { "anyVariable", "s1", "s2" },
          /* suchThatClauses */
          {
          RELATIONTUPLE{ ClauseType::FOLLOWS, { NUM_ENTITY, "1" }, { NUM_ENTITY, "2" } },
          RELATIONTUPLE{ ClauseType::MODIFIES, { PROC_SYNONYM, "procedure" }, { VAR_SYNONYM, "anyVariable" } },
          },
          /* patternClauses */ { PATTERNTUPLE{ "a", "a", "_" }, PATTERNTUPLE{ "a", "_", "_a+s*l/d+k-j_" }, PATTERNTUPLE{ "testingisgood", "greedisgood", "(expressionInBrackets)" } }) ==
    Query(/* declarationMap */ { { "anyVariable", EntityType::VARIABLE }, { "procedure", EntityType::PROCEDURE } },
          /* synonymsToReturn */ { "anyVariable", "s1", "s2" },
          /* suchThatClauses */
          {
          RELATIONTUPLE{ ClauseType::FOLLOWS, { NUM_ENTITY, "1" }, { NUM_ENTITY, "2" } },
          RELATIONTUPLE{ ClauseType::MODIFIES, { PROC_SYNONYM, "procedure" }, { VAR_SYNONYM, "anyVariable" } },
          },
          /* patternClauses */ { PATTERNTUPLE{ "a", "a", "_" }, PATTERNTUPLE{ "a", "_", "_a+s*l/d+k-j_" }, PATTERNTUPLE{ "testingisgood", "greedisgood", "(expressionInBrackets)" } }));
}

TEST_CASE("Test query equality failure") {
    REQUIRE_FALSE(
    Query(/* declarationMap */ { { "anyVariable", EntityType::VARIABLE }, { "procedure", EntityType::PROCEDURE } },
          /* synonymsToReturn */ { "anyVariable", "s1", "s2" },
          /* suchThatClauses */
          {
          RELATIONTUPLE{ ClauseType::FOLLOWS, { NUM_ENTITY, "1" }, { NUM_ENTITY, "2" } },
          RELATIONTUPLE{ ClauseType::MODIFIES, { PROC_SYNONYM, "procedure" }, { VAR_SYNONYM, "anyVariable" } },
          },
          /* patternClauses */ { PATTERNTUPLE{ "a", "a", "_" }, PATTERNTUPLE{ "a", "_", "_a+s*l/d+k-j_" }, PATTERNTUPLE{ "testingisgood", "greedisgood", "(expressionInBrackets)" } }) ==
    Query({ /* declarationMap */ { "anyVariable", EntityType::VARIABLE } },
          /* synonymsToReturn */ { "anyVariable", "anotherVar", "YetAnotherVar" },
          /* suchThatClauses */ {},
          {
          PATTERNTUPLE{ "a", "a", "_" },
          /* patternClauses */ PATTERNTUPLE{ "a", "_", "_a+s*l/d+k-j_" },
          }));
}


} // namespace qpbackend
