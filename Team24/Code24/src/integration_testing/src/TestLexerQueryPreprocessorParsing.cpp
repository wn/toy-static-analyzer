#include "Lexer.h"
#include "QPTypes.h"
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
    qpbackend::Query expectedQuery = {
        { { "v", qpbackend::EntityType::VARIABLE } },
        { "v" },
        { { qpbackend::ClauseType::FOLLOWS, { qpbackend::NUM_ENTITY, "5" }, { qpbackend::NUM_ENTITY, "6" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that relation(_, _)") {
    std::stringstream queryString =
    std::stringstream("variable v; Select v such that Follows(_,_)");
    qpbackend::Query expectedQuery = {
        { { "v", qpbackend::EntityType::VARIABLE } },
        { "v" },
        { { qpbackend::ClauseType::FOLLOWS, { qpbackend::WILDCARD, "_" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that relation(synonym, synonym)") {
    std::stringstream queryString =
    std::stringstream("variable v; call cl; Select v such that Follows(cl,v)");
    qpbackend::Query expectedQuery = {
        { { "v", qpbackend::EntityType::VARIABLE }, { "cl", qpbackend::EntityType::CALL } },
        { "v" },
        { { qpbackend::ClauseType::FOLLOWS, { qpbackend::STMT_SYNONYM, "cl" }, { qpbackend::VAR_SYNONYM, "v" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that relation(synonym, synonym) autotester format") {
    std::stringstream queryString =
    std::stringstream("procedure s, w;\nSelect s such that Follows(s,w)");
    qpbackend::Query expectedQuery = {
        { { "s", qpbackend::EntityType::PROCEDURE }, { "w", qpbackend::EntityType::PROCEDURE } },
        { "s" },
        { { qpbackend::ClauseType::FOLLOWS, { qpbackend::PROC_SYNONYM, "s" }, { qpbackend::PROC_SYNONYM, "w" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test such that clause handles whitespace") {
    std::stringstream queryString =
    std::stringstream("variable\fv;\n\n call\tcl; Select \n\n\n\nv \t\r\n\n\n such\rthat  "
                      "Follows\n(\n\n cl , \n\n\nv)\n\n\n\n\n");
    qpbackend::Query expectedQuery = {
        { { "v", qpbackend::EntityType::VARIABLE }, { "cl", qpbackend::EntityType::CALL } },
        { "v" },
        { { qpbackend::ClauseType::FOLLOWS, { qpbackend::STMT_SYNONYM, "cl" }, { qpbackend::VAR_SYNONYM, "v" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Follows

TEST_CASE("Test while w; if ifs; Select w such that Follows(w, ifs)") {
    std::stringstream queryString =
    std::stringstream("while w; if ifs; Select w such that Follows(w, ifs)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "ifs", qpbackend::EntityType::IF } },
        { "w" },
        { { qpbackend::ClauseType::FOLLOWS, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::STMT_SYNONYM, "ifs" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test stmt s; Select s such that Follows* (6, s)") {
    std::stringstream queryString = std::stringstream("stmt s; Select s such that Follows* (6, s)");
    qpbackend::Query expectedQuery = {
        {
        { "s", qpbackend::EntityType::STMT },
        },
        { "s" },
        { { qpbackend::ClauseType::FOLLOWST, { qpbackend::NUM_ENTITY, "6" }, { qpbackend::STMT_SYNONYM, "s" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Parent

TEST_CASE("Test assign a; while w; Select a such that Parent* (w, a)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Parent* (w, a)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::PARENTT, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::STMT_SYNONYM, "a" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Parent (w, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Parent (w, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::PARENT, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Uses

// Test UsesP  ... ‘(’ entRef ‘,’ entRef ‘)’

TEST_CASE("Test UsesP procedure p; variable v; Select p such that Uses(p, v)") {
    std::stringstream queryString =
    std::stringstream("procedure p; variable v; Select p such that Uses(p, v)");
    qpbackend::Query expectedQuery = {
        { { "p", qpbackend::EntityType::PROCEDURE }, { "v", qpbackend::EntityType::VARIABLE } },
        { "p" },
        { { qpbackend::ClauseType::USES, { qpbackend::PROC_SYNONYM, "p" }, { qpbackend::VAR_SYNONYM, "v" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Uses(w, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Uses(w, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::USES, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Uses(\"ident\", _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Uses(\"ident\", _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::USES, { qpbackend::NAME_ENTITY, "ident" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Uses(\"ident\", \"ident\")") {
    std::stringstream queryString =
    std::stringstream(R"(assign a; while w; Select a such that Uses("hello", "there"))");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::USES, { qpbackend::NAME_ENTITY, "hello" }, { qpbackend::NAME_ENTITY, "there" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test UsesS  ... ‘(’ stmtRef ‘,’ entRef ‘)’

TEST_CASE("Test assign a; while w; Select a such that Uses(8, \"ident\")") {
    std::stringstream queryString =
    std::stringstream(R"(assign a; while w; Select a such that Uses(8, "there"))");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::USES, { qpbackend::NUM_ENTITY, "8" }, { qpbackend::NAME_ENTITY, "there" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Uses(8, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Uses(8, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::USES, { qpbackend::NUM_ENTITY, "8" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Uses(8, a)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Uses(8, a)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::USES, { qpbackend::NUM_ENTITY, "8" }, { qpbackend::STMT_SYNONYM, "a" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}


// Test ModifiesP  ... ‘(’ entRef ‘,’ entRef ‘)’

TEST_CASE("Test UsesP procedure p; variable v; Select p such that Modifies(p, v)") {
    std::stringstream queryString =
    std::stringstream("procedure p; variable v; Select p such that Modifies(p, v)");
    qpbackend::Query expectedQuery = {
        { { "p", qpbackend::EntityType::PROCEDURE }, { "v", qpbackend::EntityType::VARIABLE } },
        { "p" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::PROC_SYNONYM, "p" }, { qpbackend::VAR_SYNONYM, "v" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Modifies(w, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Modifies(w, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Modifies(\"ident\", _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Modifies(\"ident\", _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::NAME_ENTITY, "ident" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Modifies(\"ident\", \"ident\")") {
    std::stringstream queryString =
    std::stringstream(R"(assign a; while w; Select a such that Modifies("hello", "there"))");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::NAME_ENTITY, "hello" }, { qpbackend::NAME_ENTITY, "there" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test ModifiesS  ... ‘(’ stmtRef ‘,’ entRef ‘)’

TEST_CASE("Test assign a; while w; Select a such that Modifies(8, \"ident\")") {
    std::stringstream queryString =
    std::stringstream(R"(assign a; while w; Select a such that Modifies(8, "there"))");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::NUM_ENTITY, "8" }, { qpbackend::NAME_ENTITY, "there" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Modifies(8, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Modifies(8, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::NUM_ENTITY, "8" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test assign a; while w; Select a such that Modifies(8, a)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Modifies(8, a)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::MODIFIES, { qpbackend::NUM_ENTITY, "8" }, { qpbackend::STMT_SYNONYM, "a" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Next ‘(’ lineRef ‘,’ lineRef ‘)’
TEST_CASE("Test Next(synonym, synonym)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Next(a, w)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::NEXT, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "w" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test Next(_, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Next(_, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::NEXT, { qpbackend::WILDCARD, "_" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test Next(INTEGER, INTEGER)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Next(1, 2)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::NEXT, { qpbackend::NUM_ENTITY, "1" }, { qpbackend::NUM_ENTITY, "2" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Next* ‘(’ lineRef ‘,’ lineRef ‘)’
TEST_CASE("Test Next*(synonym, synonym)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Next*(a, w)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::NEXTT, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "w" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test Next*(_, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Next*(_, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::NEXTT, { qpbackend::WILDCARD, "_" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test Next*(INTEGER, INTEGER)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Next*(1, 2)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::NEXTT, { qpbackend::NUM_ENTITY, "1" }, { qpbackend::NUM_ENTITY, "2" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Calls ‘(’ entRef ‘,’ entRef ‘)’
TEST_CASE("Test Calls(synonym, synonym)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Calls(a, w)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::CALLS, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "w" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test Calls(_, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Calls(_, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::CALLS, { qpbackend::WILDCARD, "_" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}


TEST_CASE("Test Calls(IDENT, IDENT)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Calls(\"hello\", \"there\")");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::CALLS, { qpbackend::NAME_ENTITY, "hello" }, { qpbackend::NAME_ENTITY, "there" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test Calls* ‘(’ entRef ‘,’ entRef ‘)’
TEST_CASE("Test Calls*(synonym, synonym)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Calls*(a, w)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::CALLST, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "w" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test Calls*(_, _)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Calls*(_, _)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::CALLST, { qpbackend::WILDCARD, "_" }, { qpbackend::WILDCARD, "_" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}


TEST_CASE("Test Calls*(IDENT, IDENT)") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Calls*(\"hello\", \"there\")");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::CALLST, { qpbackend::NAME_ENTITY, "hello" }, { qpbackend::NAME_ENTITY, "there" } } },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test multiple such that clauses

TEST_CASE("Test multiple such that clauses.") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a such that Parent(w, _) such that Parent (w,_) "
                      "such that Parent*(w,_) such that Follows*(a,a) such that Follows(2,_)");
    qpbackend::Query expectedQuery = {
        { { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        {
        { qpbackend::ClauseType::PARENT, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::WILDCARD, "_" } },
        { qpbackend::ClauseType::PARENT, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::WILDCARD, "_" } },
        { qpbackend::ClauseType::PARENTT, { qpbackend::STMT_SYNONYM, "w" }, { qpbackend::WILDCARD, "_" } },
        { qpbackend::ClauseType::FOLLOWST, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "a" } },
        { qpbackend::ClauseType::FOLLOWS, { qpbackend::NUM_ENTITY, "2" }, { qpbackend::WILDCARD, "_" } },
        },
        {}
    };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test patterns clause

TEST_CASE("Test basic pattern clause") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a pattern a (\"v\", _)");
    qpbackend::Query expectedQuery = { { { "w", qpbackend::EntityType::WHILE },
                                         { "a", qpbackend::EntityType::ASSIGN } },
                                       { "a" },
                                       {},
                                       { { "a", "\"v\"", "_" } } };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Note that QPP now validates to see if synonyms are declared before hand.
TEST_CASE("Test basic pattern synonym clause") {
    std::stringstream queryString =
    std::stringstream("assign a; while w; Select a pattern a (w, _)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "w", qpbackend::EntityType::WHILE }, { "a", qpbackend::EntityType::ASSIGN } },
                     { "a" }, {}, { { "a", "w", "_" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause wild cards") {
    std::stringstream queryString = std::stringstream("assign a; Select a pattern a (_, _)");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" }, {}, { { "a", "_", "_" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause expression") {
    std::stringstream queryString =
    std::stringstream("assign a; Select a pattern a (_, \"x+s+Follows*38\")");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" },
                                                      {}, { { "a", "_", "\"x+s+Follows*38\"" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause expression operator") {
    std::stringstream queryString =
    std::stringstream("assign a; Select a pattern a (_, \"x+s*g-3%9\")");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } },
                                                      { "a" }, {}, { { "a", "_", "\"x+s*g-3%9\"" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause expression white space handling") {
    std::stringstream queryString =
    std::stringstream("assign a; Select a pattern a (_, \"x\r\r\r+s+Follows*\n\r3 8\")");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" },
                                                      {}, { { "a", "_", "\"x +s+Follows* 3 8\"" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause sub-expression") {
    std::stringstream queryString =
    std::stringstream("assign a; Select a pattern a (_, _\"x+s+Follows*38\"_)");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" },
                                                      {}, { { "a", "_", "_\"x+s+Follows*38\"_" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause sub-expression white space handling") {
    std::stringstream queryString = std::stringstream(
    "assign a; Select a pattern a (_, _   \r \"x+s\t+\tFollows       \t*  \n38\"    \t_)");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" },
                                                      {}, { { "a", "_", "_\"x+s + Follows * 38\"_" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test valid QPP queries but "invalid" at PKB query level.

TEST_CASE("Test pattern clause EXPR valid QPP but invalid PKB") {
    std::stringstream queryString = std::stringstream("assign a; Select a pattern a (_, \"()\" )");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" }, {}, { { "a", "_", "\"()\"" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern clause EXPR valid QPP but invalid PKB unbalanced round brackets") {
    std::stringstream queryString =
    std::stringstream("assign a; Select a pattern a (_, \"   (( \" )");
    qpbackend::Query expectedQuery = qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } },
                                                      { "a" }, {}, { { "a", "_", "\"(( \"" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test multiple patterns

TEST_CASE("Test multiple pattern clause") {
    std::stringstream queryString =
    std::stringstream("assign a; Select a pattern a (_, _\"x+s+Follows*38\"_)pattern a (_, "
                      "_\"x+s+Follows*38\"_)pattern a (_, _\"x+s+Follows*38\"_)    ");
    qpbackend::Query expectedQuery =
    qpbackend::Query({ { "a", qpbackend::EntityType::ASSIGN } }, { "a" }, {},
                     { { "a", "_", "_\"x+s+Follows*38\"_" },
                       { "a", "_", "_\"x+s+Follows*38\"_" },
                       { "a", "_", "_\"x+s+Follows*38\"_" } });

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

// Test such-that and pattern clauses

TEST_CASE("Test such-that and pattern clause") {
    std::stringstream queryString = std::stringstream(
    "assign a; Select a such that Follows*(a,a) pattern a (_, _\"x+s+Follows*38\"_)    ");
    qpbackend::Query expectedQuery = { { { "a", qpbackend::EntityType::ASSIGN } },
                                       { "a" },
                                       { { qpbackend::ClauseType::FOLLOWST,
                                           { qpbackend::STMT_SYNONYM, "a" },
                                           { qpbackend::STMT_SYNONYM, "a" } } },
                                       { { "a", "_", "_\"x+s+Follows*38\"_" } } };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test pattern and such-that clause") {
    std::stringstream queryString = std::stringstream(
    "assign a; Select a pattern a (_, _\"x+s+Follows*38\"_) such that  Follows*(a,a)   ");
    qpbackend::Query expectedQuery = { { { "a", qpbackend::EntityType::ASSIGN } },
                                       { "a" },
                                       { { qpbackend::ClauseType::FOLLOWST,
                                           { qpbackend::STMT_SYNONYM, "a" },
                                           { qpbackend::STMT_SYNONYM, "a" } } },
                                       { { "a", "_", "_\"x+s+Follows*38\"_" } } };

    std::vector<lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(queryString);
    qpbackend::Query actualQuery = querypreprocessor::parseTokens(lexerTokens);

    REQUIRE(expectedQuery == actualQuery);
}

TEST_CASE("Test multiple pattern and such-that clause") {
    std::stringstream queryString = std::stringstream(
    "assign a; Select a such that  Follows*(a,a) pattern a (_, _\"x+s+Follows*38\"_) pattern a (_, "
    "_\"x+s+Follows*38\"_) such that  Follows*(a,a)   ");
    qpbackend::Query expectedQuery = {
        { { "a", qpbackend::EntityType::ASSIGN } },
        { "a" },
        { { qpbackend::ClauseType::FOLLOWST, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "a" } },
          { qpbackend::ClauseType::FOLLOWST, { qpbackend::STMT_SYNONYM, "a" }, { qpbackend::STMT_SYNONYM, "a" } } },
        { { "a", "_", "_\"x+s+Follows*38\"_" }, { "a", "_", "_\"x+s+Follows*38\"_" } }
    };

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

TEST_CASE("Test Uses first argument '_' failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("variable v; Select v such that Uses(_,v)");
}

TEST_CASE("Test Modifies first argument '_' failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("variable v; Select v such that Modifies(_,v)");
}

TEST_CASE("Test non syn-assign used as syn-assign failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("variable v; Select v pattern v (_,_)");
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
    std::stringstream queryString = std::stringstream("adf;oij32!@d v");
    REQUIRE_THROWS(backend::lexer::tokenizeWithWhitespace(queryString));
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

TEST_CASE("Test such that relation invalid such failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "stmt s1T2U3y4; variable v; assign a; while w; if ifs;\nSelect s1T2U3y4 pattern a(v, _\"1\"_) "
    "suhc that Follows*(s1T2U3y4, w)");
}

TEST_CASE("Test such that relation invalid that failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "stmt s1T2U3y4; variable v; assign a; while w; if ifs;\nSelect s1T2U3y4 pattern a(v, _\"1\"_) "
    "such tht Follows*(s1T2U3y4, w)");
}

TEST_CASE("Test star relations whitespace sensitivity causing invalid query (Parent *)") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("stmt s; Select s such that Parent * (6, s)");
}

TEST_CASE("Test star relations whitespace sensitivity causing invalid query (Follows *)") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery("stmt s; Select s such that Follows * (6, s)");
}

// Syntax error detection Pattern-cl

TEST_CASE("Test pattern and such-that clause invalid keywords failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _\"x+s+Follows*38\"_)   Follows*(a,a)   ");
}

TEST_CASE("Test pattern invalid expression spec (\"\") failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, \"\")");
}

TEST_CASE("Test pattern invalid expression spec (_\"\") failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _\"\")");
}

TEST_CASE("Test pattern invalid expression spec (__) failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, __)");
}

TEST_CASE("Test pattern invalid expression spec (_\"\"_) failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _\"\"_)");
}

TEST_CASE("Test pattern invalid expression spec missing both DOUBLE_QUOTES failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, 1+1)");
}

TEST_CASE("Test pattern invalid expression spec missing closing DOUBLE_QUOTES failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, \"1+1)");
}

TEST_CASE("Test pattern invalid expression spec missing starting DOUBLE_QUOTES failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, 1+1\")");
}
/** Ignored test cases
 * TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/235):
 * Bug in lexer causing exceptions to be thrown.
TEST_CASE("Test pattern invalid expression spec (sub expr) missing both DOUBLE_QUOTES failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _1+1_)");
}

TEST_CASE("Test pattern invalid expression spec (sub expr) missing closing DOUBLE_QUOTES failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _\"1+1_)");
}
**/

TEST_CASE("Test pattern invalid expression spec (sub expr) missing starting DOUBLE_QUOTES "
          "failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _1+1\"_)");
}

TEST_CASE("Test pattern invalid expression spec (sub expr), missing closing UNDER_SCORE failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, _\"1+1\")");
}

TEST_CASE("Test pattern invalid expression spec (sub expr), missing starting UNDER_SCORE failure") {
    requireParsingInvalidQPLQueryToReturnEmptyQuery(
    "assign a; Select a such that pattern a (_, \"1+1\"_)");
}

} // namespace backend
