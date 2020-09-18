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

} // namespace qpbackend
