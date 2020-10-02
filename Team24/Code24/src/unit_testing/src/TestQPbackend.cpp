#include "QPTypes.h"
#include "catch.hpp"

#include <Query.h>

namespace qpbackend {
// Test the following mappings
// design-entity : ‘stmt’ | ‘read’ | ‘print’ | ‘call’ | ‘while’ | ‘if’ | ‘assign’ | ‘variable’ | ‘constant’ | ‘procedure’
TEST_CASE("Test entityTypeFromString mappings") {
    REQUIRE(EntityType::STMT == entityTypeFromString("stmt"));
    REQUIRE(EntityType::PRINT == entityTypeFromString("print"));
    REQUIRE(EntityType::WHILE == entityTypeFromString("while"));
    REQUIRE(EntityType::ASSIGN == entityTypeFromString("assign"));
    REQUIRE(EntityType::CONSTANT == entityTypeFromString("constant"));
    REQUIRE(EntityType::READ == entityTypeFromString("read"));
    REQUIRE(EntityType::CALL == entityTypeFromString("call"));
    REQUIRE(EntityType::IF == entityTypeFromString("if"));
    REQUIRE(EntityType::VARIABLE == entityTypeFromString("variable"));
    REQUIRE(EntityType::PROCEDURE == entityTypeFromString("procedure"));
}

TEST_CASE("Test entityTypeFromString nonsense string throws") {
    REQUIRE_THROWS(entityTypeFromString("isberget"));
}

TEST_CASE("Test entityTypeFromString throws") {
    REQUIRE_THROWS(entityTypeFromString("Stmt"));
}

// Test the keywords from the following mappings
// relRef : Follows | FollowsT | Parent | ParentT | UsesS | UsesP | ModifiesS | ModifiesP
// UsesS and UsesP share the same keyword
// ModifiesS and ModifiesP share the same keyword
TEST_CASE("Test relationClauseTypeFromString mappings") {
    REQUIRE(ClauseType::FOLLOWS == relationClauseTypeFromString("Follows"));
    REQUIRE(ClauseType::FOLLOWST == relationClauseTypeFromString("Follows*"));
    REQUIRE(ClauseType::PARENT == relationClauseTypeFromString("Parent"));
    REQUIRE(ClauseType::PARENTT == relationClauseTypeFromString("Parent*"));
    REQUIRE(ClauseType::USES == relationClauseTypeFromString("Uses"));
    REQUIRE(ClauseType::MODIFIES == relationClauseTypeFromString("Modifies"));
}

TEST_CASE("Test relationClauseTypeFromString nonsense string throws") {
    REQUIRE_THROWS(entityTypeFromString("isberget"));
}

TEST_CASE("Test relationClauseTypeFromString case sensitivity") {
    REQUIRE_THROWS(entityTypeFromString("follows"));
}

TEST_CASE("Test isRelationClauseString") {
    REQUIRE(isRelationClauseString("Follows"));
    REQUIRE(isRelationClauseString("Follows*"));
    REQUIRE(isRelationClauseString("Parent"));
    REQUIRE(isRelationClauseString("Parent*"));
    REQUIRE(isRelationClauseString("Uses"));
    REQUIRE(isRelationClauseString("Modifies"));
}

TEST_CASE("Test isRelationClauseString nonsense string") {
    REQUIRE_FALSE(isRelationClauseString("fasjf"));
}

TEST_CASE("Test isRelationClauseString case sensitivity") {
    REQUIRE_FALSE(isRelationClauseString("parent"));
    REQUIRE_FALSE(isRelationClauseString("ParenT"));
}

} // namespace qpbackend
