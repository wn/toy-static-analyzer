#include "Logger.h"
#include "TNode.h"

#include <stack>
#include <unordered_map>

using namespace std;

namespace backend {
namespace extractor {

std::unordered_map<TNode, int> getTNodeToStatementNumber(const TNode& ast) {
    logLine("start getTNodeToStatementNumber");
    logWord("getTNodeToStatementNumber: ast is ");
    logLine(ast.toString());

    std::unordered_map<TNode, int> tNodeToStatementNumber;
    int currentStatementNumber = 1;

    // DFS through the AST and assign statement numbers.
    // Here, we implicitly assume that AST children are ordered by their
    // beginning line number in the source code. For example, we assume that the
    // "then" statement list is a child
    std::stack<TNode> nodeStack;
    nodeStack.push(ast);
    while (!nodeStack.empty()) {
        const TNode tNode = nodeStack.top();
        nodeStack.pop();

        logWord("Popped tNode from stack: ");
        logLine(tNode.toShortString());

        if (tNode.isStatementNode()) {

            logWord("Found Statement tNode");
            logWord(tNode.toShortString());
            logWord(", assigning");
            logLine(currentStatementNumber);
            logWord("  Does this tNode already exist in the Hashmap?");
            logLine(tNodeToStatementNumber.count(tNode));

            tNodeToStatementNumber[tNode] = currentStatementNumber;
            currentStatementNumber++;
        }


        logWord(" number of children: ");
        logLine(tNode.children.size());
        // Iterate in the reverse direction to ensure that the top of the stack is the
        // "first" statement in the SIMPLE program
        for (int childIndex = (int)tNode.children.size() - 1; childIndex >= 0; childIndex--) {
            logWord("  childindex ");
            logLine(childIndex);

            const TNode& child = tNode.children[childIndex];

            logWord("  Adding ");
            logLine(child.toShortString());

            nodeStack.push(child);
        }
    }

    logLine("success getTNodeToStatementNumber");
    return tNodeToStatementNumber;
}

std::unordered_map<TNodeType, std::vector<TNode*>, EnumClassHash> getTNodeTypeToTNodes(TNode& ast) {
    std::unordered_map<TNodeType, std::vector<TNode*>, EnumClassHash> typesToTNode;
    std::vector<TNode*> toVisit;
    toVisit.push_back(&ast);
    while (toVisit.size() > 0) {
        TNode* visiting = toVisit.back();
        TNodeType type = visiting->type;
        if (typesToTNode.find(type) == typesToTNode.end()) {
            typesToTNode[type] = {};
        }
        typesToTNode[type].push_back(visiting);

        toVisit.pop_back();
        for (TNode& c : visiting->children) {
            toVisit.push_back(&c);
        }
    }
    return typesToTNode;
}

std::unordered_map<int, TNode> getStatementNumberToTNode(const std::unordered_map<TNode, int>& tNodeToStatementNumber) {
    std::unordered_map<int, TNode> statementNumberToTNode;
    for (const auto& p : tNodeToStatementNumber) {
        statementNumberToTNode[p.second] = p.first;
    }
    return statementNumberToTNode;
}

} // namespace extractor
} // namespace backend
