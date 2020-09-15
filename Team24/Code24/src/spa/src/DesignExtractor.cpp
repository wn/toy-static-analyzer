#include <stack>
#include <unordered_map>

#include <Logger.h>
#include <TNode.h>

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

std::unordered_map<int, TNode> getStatementNumberToTNode(const std::unordered_map<TNode, int>& tNodeToStatementNumber) {
    std::unordered_map<int, TNode> statementNumberToTNode;
    for (const auto& p : tNodeToStatementNumber) {
        statementNumberToTNode[p.second] = p.first;
    }
    return statementNumberToTNode;
}

} // namespace extractor
} // namespace backend
