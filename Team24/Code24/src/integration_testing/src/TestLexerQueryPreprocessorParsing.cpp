#include "Lexer.h"
#include "Query.h"
#include "QueryPreprocessor.h"
#include "catch.hpp"

#include <sstream>
#include <unordered_map>


namespace backend {

// Helper functions

void requireParsingInvalidQPLQueryToReturnEmptyQuery(const std::string& s) {
    std::stringstream queryString = std::stringstream(s);

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);

    REQUIRE(qpbackend::Query() == querypreprocessor::parseTokens(lexerTokens));
}


// select-cl : declaration ‘Select’ synonym

TEST_CASE("Test selects clause without pattern or such that clause") {
    std::stringstream queryString = std::stringstream("variable v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::VARIABLE } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test keywords are not reserved") {
    std::unordered_map<std::string, qpbackend::EntityType> expectedDeclarationMap;
    std::stringstream queryString;
    std::vector<std::string> keywords = {
        "stmt",     "read",     "print",     "call",      "while",    "if",      "assign",
        "variable", "constant", "procedure", "prog_line", "procName", "varName", "value",
        "such",     "that",     "with",      "and",       "Modifies", "Uses",    "Calls",
        "Parent",   "Follows",  "Next",      "Affects",   "pattern",
    };
    queryString << "variable ";
    for (const auto& keyword : keywords) {
        expectedDeclarationMap.insert(
        std::pair<std::string, qpbackend::EntityType>(keyword, qpbackend::EntityType::VARIABLE));
        queryString << keyword;
        if (keyword == keywords.back()) {
            queryString << ";";
        } else {
            queryString << ", ";
        }
    }
    queryString << " Select pattern";
    qpbackend::Query expectedQuery = qpbackend::Query(expectedDeclarationMap, { "pattern" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// select-cl : declaration ‘Select’ synonym suchthat-cl

// General grammar tests

TEST_CASE("Test such that relation(INTEGER, INTEGER)") {
    std::stringstream queryString =
    std::stringstream("variable v; Select v such that Follows(5,6)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::VARIABLE } }, { "v" },
                     { { qpbackend::RelationType::FOLLOWS, "5", "6" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that relation(_, _)") {
    std::stringstream queryString =
    std::stringstream("variable v; Select v such that Follows(_,_)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::VARIABLE } }, { "v" },
                     { { qpbackend::RelationType::FOLLOWS, "_", "_" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that relation(synonym, synonym)") {
    std::stringstream queryString =
    std::stringstream("variable v; call cl; Select v such that Follows(cl,v)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::VARIABLE }, { "cl", qpbackend::EntityType::CALL } },
                     { "v" }, { { qpbackend::RelationType::FOLLOWS, "cl", "v" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that clause handles whitespace") {
    std::stringstream queryString =
    std::stringstream("variable\fv;\n\n call\tcl; Select \n\n\n\nv \t\r\n\n\n such\rthat  "
                      "Follows\n(\n\n cl , \n\n\nv)\n\n\n\n\n");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::VARIABLE }, { "cl", qpbackend::EntityType::CALL } },
                     { "v" }, { { qpbackend::RelationType::FOLLOWS, "cl", "v" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Follows

TEST_CASE("Test while w; if ifs; Select w such that Follows(w, ifs)") {
    std::stringstream queryString =
    std::stringstream("while w; if ifs; Select w such that Follows(w, ifs)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "w", qpbackend::EntityType::WHILE }, { "ifs", qpbackend::EntityType::IF } },
                     { "w" }, { { qpbackend::RelationType::FOLLOWS, "w", "ifs" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test stmt s; Select s such that Follows* (6, s)") {
    std::stringstream queryString = std::stringstream("stmt s; Select s such that Follows* (6, s)");
    qpbackend::Query expectedQuery = qpbackend::Query(
    {
    { "s", qpbackend::EntityType::STMT },
    },
    { "s" }, { { qpbackend::RelationType::FOLLOWST, "6", "s" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Parent

TEST_CASE("Test assign a; while w; Select a such that Parent* (w, a)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Parent* (w, a)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
                     { "a" }, { { qpbackend::RelationType::PARENTT, "w", "a" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Parent (w, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Parent (w, _)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
                     { "a" }, { { qpbackend::RelationType::PARENT, "w", "_" } }, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test multiple such that clauses

TEST_CASE("Test multiple such that clauses.") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Parent(w, _) such that Parent (w,_) "
                      "such that Parent*(w,_) such that Follows*(a,a) such that Follows(2,_)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } }, { "a" },
                     {
                     { qpbackend::RelationType::PARENT, "w", "_" },
                     { qpbackend::RelationType::PARENT, "w", "_" },
                     { qpbackend::RelationType::PARENTT, "w", "_" },
                     { qpbackend::RelationType::FOLLOWST, "a", "a" },
                     { qpbackend::RelationType::FOLLOWS, "2", "_" },
                     },
                     {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}


// Test multiple declarations

TEST_CASE("Test multiple declarations, single synonym each") {
    std::stringstream queryString = std::stringstream("procedure v; assign a; stmt s; Select v");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "v", qpbackend::EntityType::PROCEDURE },
                                                        { "a", qpbackend::EntityType::ASSIGN },
                                                        { "s", qpbackend::EntityType::STMT } },
                                                      { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test multiple declarations, multiple synonyms") {
    std::stringstream queryString =
    std::stringstream("procedure v, v2; assign a, a2, a3 ,a4; stmt s; Select v");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "v", qpbackend::EntityType::PROCEDURE },
                                                        { "v2", qpbackend::EntityType::PROCEDURE },
                                                        { "a", qpbackend::EntityType::ASSIGN },
                                                        { "a2", qpbackend::EntityType::ASSIGN },
                                                        { "a3", qpbackend::EntityType::ASSIGN },
                                                        { "a4", qpbackend::EntityType::ASSIGN },
                                                        { "s", qpbackend::EntityType::STMT } },
                                                      { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test parsing synonym declarations of all entity types
// design-entity : ‘stmt’ | ‘read’ | ‘print’ | ‘call’ | ‘while’ | ‘if’ | ‘assign’ | ‘variable’ | ‘constant’ | ‘procedure’

TEST_CASE("Test declaring stmt") {
    std::stringstream queryString = std::stringstream("stmt v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::STMT } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring read") {
    std::stringstream queryString = std::stringstream("read v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::READ } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring print") {
    std::stringstream queryString = std::stringstream("print v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::PRINT } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring call") {
    std::stringstream queryString = std::stringstream("call v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::CALL } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring while") {
    std::stringstream queryString = std::stringstream("while v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::WHILE } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring if") {
    std::stringstream queryString = std::stringstream("if v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::IF } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring assign") {
    std::stringstream queryString = std::stringstream("assign v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::ASSIGN } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring variable") {
    std::stringstream queryString = std::stringstream("variable v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::VARIABLE } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring constant") {
    std::stringstream queryString = std::stringstream("constant v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::CONSTANT } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test declaring procedure") {
    std::stringstream queryString = std::stringstream("procedure v; Select v");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "v", qpbackend::EntityType::PROCEDURE } }, { "v" }, {}, {});

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test semantic error detection

TEST_CASE("Test redeclaration failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("procedure v, v; Select v");
}

TEST_CASE("Test selected synonyms missing failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("procedure a; Select v");
}

TEST_CASE("Test selected synonyms missing due to no declaration failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("Select v");
}

// Test syntax error detection

TEST_CASE("Test empty query failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("");
}

TEST_CASE("Test clauses wrong order failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("Select v; variable v");
}

TEST_CASE("Test missing Select keyword failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("procedure v; v");
}

TEST_CASE("Test declaration missing delimiter failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("procedure v a;select v");
}

TEST_CASE("Test declaration wrong case failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("procedure v;select v");
}


TEST_CASE("Test parsing nonsense failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("adf;oij32!@d v");
}

TEST_CASE("Test parsing invalid variable name (Integer)") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("stmt 999; Select 999");
}

TEST_CASE("Test parsing invalid variable name (Expresion)") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(R"(stmt "x"; Select "x")");
}

// Test syntax error detection (suchthat-cl)

TEST_CASE("Test parsing invalid number of arguments (<2) failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; while w; Select a such that Parent* (w)");
}

TEST_CASE("Test parsing invalid number of arguments (>2) failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; while w; Select a such that Parent* (w, a, w)");
}

TEST_CASE("Test parsing missing relation keyword failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("assign a; while w; Select a such that (w, a)");
}

TEST_CASE("Test parsing wrong case relation keyword failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; while w; Select a such that pArEnT(w, a)");
}

TEST_CASE("Test parsing relation with whitespace separating keyword failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; while w; Select a such that pArEnT(w, a)");
}

TEST_CASE("Test parsing non-existing relation keyword failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; while w; Select a such that Patrick*(w, a)");
}

TEST_CASE("Test star relations whitespace sensitivity causing invalid query (Parent *)") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("stmt s; Select s such that Parent * (6, s)");
}

TEST_CASE("Test star relations whitespace sensitivity causing invalid query (Follows *)") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("stmt s; Select s such that Follows * (6, s)");
}

} // namespace backend
