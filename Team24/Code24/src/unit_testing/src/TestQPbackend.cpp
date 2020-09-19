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
TEST_CASE("Test relationTypeFromString mappings") {
    REQUIRE(RelationType::FOLLOWS == relationTypeFromString("Follows"));
    REQUIRE(RelationType::FOLLOWST == relationTypeFromString("Follows*"));
    REQUIRE(RelationType::PARENT == relationTypeFromString("Parent"));
    REQUIRE(RelationType::PARENTT == relationTypeFromString("Parent*"));
    REQUIRE(RelationType::USES == relationTypeFromString("Uses"));
    REQUIRE(RelationType::MODIFIES == relationTypeFromString("Modifies"));
}

TEST_CASE("Test relationTypeFromString nonsense string throws") {
    REQUIRE_THROWS(entityTypeFromString("isberget"));
}

TEST_CASE("Test relationTypeFromString case sensitivity") {
    REQUIRE_THROWS(entityTypeFromString("follows"));
}

TEST_CASE("Test isRelationString") {
    REQUIRE(isRelationString("Follows"));
    REQUIRE(isRelationString("Follows*"));
    REQUIRE(isRelationString("Parent"));
    REQUIRE(isRelationString("Parent*"));
    REQUIRE(isRelationString("Uses"));
    REQUIRE(isRelationString("Modifies"));
}

TEST_CASE("Test isRelationString nonsense string") {
    REQUIRE_FALSE(isRelationString("fasjf"));
}

TEST_CASE("Test isRelationString case sensitivity") {
    REQUIRE_FALSE(isRelationString("parent"));
    REQUIRE_FALSE(isRelationString("ParenT"));
}

} // namespace qpbackend
