#include "Parser.h"

#include "catch.hpp"
namespace backend {
namespace testparser {
void require(bool b) {
    REQUIRE(b);
}

Parser GenerateParserFromTokens(const std::string& expr) {
    std::istringstream iStr(expr);
    return Parser(lexer::tokenize(iStr));
}

TNode generateProgramNodeFromStatement(const std::string& name, const TNode& node) {
    TNode stmtNode(TNodeType::StatementList, 1);
    stmtNode.addChild(node);

    TNode procNode(TNodeType::Procedure, 1);
    procNode.name = name;
    procNode.addChild(stmtNode);

    TNode progNode(TNodeType::Program, -1);
    progNode.addChild(procNode);
    return progNode;
}

TEST_CASE("Test parseEmptyProcedure") {
    Parser parser = GenerateParserFromTokens("procedure p{}");
    TNode result = parser.parse();

    TNode stmtNode(TNodeType::StatementList, 1);
    TNode procNode(TNodeType::Procedure, 1);
    procNode.name = "p";
    procNode.addChild(stmtNode);

    TNode progNode(TNodeType::Program, -1);
    progNode.addChild(procNode);
    require(result == progNode);
}
} // namespace testparser
} // namespace backend
