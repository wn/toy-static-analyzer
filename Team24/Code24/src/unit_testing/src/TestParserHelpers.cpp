#include "TestParserHelpers.h"

#include "catch.hpp"

namespace backend {
namespace testhelpers {

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
    procNode.name = "p";
    procNode.addChild(stmtNode);

    TNode progNode(TNodeType::Program, 0);
    progNode.addChild(procNode);
    return progNode;
}

// create an assign TNode for `y=y+1`.
TNode generateMockAssignNode() {
    TNode y(Variable, 1);
    y.name = "y";

    TNode constNode(Constant, 1);
    constNode.constant = "1";

    TNode plusNode(Plus);
    plusNode.addChild(y);
    plusNode.addChild(constNode);

    TNode assignNode(Assign, 1);
    assignNode.addChild(y);
    assignNode.addChild(plusNode);

    return assignNode;
}
// Procedure node name is p
TNode generateProgramNodeFromCondition(const TNode& node) {
    TNode stmt = generateMockAssignNode();

    TNode innerStmtNode(TNodeType::StatementList, 1);
    innerStmtNode.addChild(stmt);

    TNode whileNode(TNodeType::While, 1);
    whileNode.addChild(node); // condition
    whileNode.addChild(innerStmtNode); // add dummy assign tnode to while-loop
    TNode stmtNode(TNodeType::StatementList, 1);
    stmtNode.addChild(whileNode);

    TNode procNode(TNodeType::Procedure, 1);
    procNode.name = "p";
    procNode.addChild(stmtNode);

    TNode progNode(TNodeType::Program, 0);
    progNode.addChild(procNode);
    return progNode;
}

} // namespace testhelpers
} // namespace backend
